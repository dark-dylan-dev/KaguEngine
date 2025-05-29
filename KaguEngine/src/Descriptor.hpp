#pragma once

#include "Device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace KaguEngine {

class DescriptorSetLayout {
public:
    class Builder {
    public:
        explicit Builder(Device &Device) : deviceRef{Device} {}

        Builder &addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags,
                            uint32_t count = 1);
        std::unique_ptr<DescriptorSetLayout> build() const;

    private:
        Device &deviceRef;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_BuilderBindings{};
    };

    DescriptorSetLayout(Device &m_Device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
    ~DescriptorSetLayout();
    DescriptorSetLayout(const DescriptorSetLayout &) = delete;
    DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

    VkDescriptorSetLayout getDescriptorSetLayout() const { return m_DescriptorSetLayout; }

private:
    Device &deviceRef;
    VkDescriptorSetLayout m_DescriptorSetLayout;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_Bindings;

    friend class DescriptorWriter;
};

class DescriptorPool {
public:
    class Builder {
    public:
        explicit Builder(Device &Device) : deviceRef{Device} {}

        Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
        Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
        Builder &setMaxSets(uint32_t count);
        [[nodiscard]] std::unique_ptr<DescriptorPool> build() const;

    private:
        Device &deviceRef;
        std::vector<VkDescriptorPoolSize> poolSizes{};
        uint32_t maxSets = 1000;
        VkDescriptorPoolCreateFlags poolFlags = 0;
    };

    DescriptorPool(Device &Device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags,
                   const std::vector<VkDescriptorPoolSize> &poolSizes);
    ~DescriptorPool();
    DescriptorPool(const DescriptorPool &) = delete;
    DescriptorPool &operator=(const DescriptorPool &) = delete;

    bool allocateDescriptor(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

    void freeDescriptors(const std::vector<VkDescriptorSet> &descriptors) const;
    VkDescriptorPool& getDescriptorPool() { return m_DescriptorPool; }

    void resetPool() const;

private:
    Device &deviceRef;
    VkDescriptorPool m_DescriptorPool;

    friend class DescriptorWriter;
};

class DescriptorWriter {
public:
    DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool);

    DescriptorWriter &writeBuffer(uint32_t binding, const VkDescriptorBufferInfo *bufferInfo);
    DescriptorWriter &writeImage(uint32_t binding, const VkDescriptorImageInfo *imageInfo);

    bool build(VkDescriptorSet &set);
    void overwrite(const VkDescriptorSet &set);

private:
    DescriptorSetLayout &setLayoutRef;
    DescriptorPool &poolRef;
    std::vector<VkWriteDescriptorSet> m_Writes;
};

} // Namespace KaguEngine
