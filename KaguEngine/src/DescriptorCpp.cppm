module;

// Vulkan
#include <vulkan/vulkan.h>

// std
#include <cassert>
#include <memory>
#include <ranges>
#include <unordered_map>
#include <vector>
#include <stdexcept>

export module Descriptor;
export import :Hpp;

export import Device;

export namespace KaguEngine {

DescriptorSetLayout::Builder &DescriptorSetLayout::Builder::addBinding(const uint32_t binding,
                                                                       const VkDescriptorType descriptorType,
                                                                       const VkShaderStageFlags stageFlags,
                                                                       const uint32_t count) {
    assert(!m_BuilderBindings.contains(binding) && "Binding already in use");
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = descriptorType;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = stageFlags;
    m_BuilderBindings[binding] = layoutBinding;
    return *this;
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const {
    return std::make_unique<DescriptorSetLayout>(deviceRef, m_BuilderBindings);
}

DescriptorSetLayout::DescriptorSetLayout(Device &m_Device,
                                         std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings) :
    deviceRef{m_Device}, m_Bindings{bindings} {
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
    for (auto val: bindings | std::views::values) {
        setLayoutBindings.push_back(val);
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
    descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

    if (vkCreateDescriptorSetLayout(m_Device.device(), &descriptorSetLayoutInfo, nullptr, &m_DescriptorSetLayout) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

DescriptorSetLayout::~DescriptorSetLayout() {
    vkDestroyDescriptorSetLayout(deviceRef.device(), m_DescriptorSetLayout, nullptr);
}

DescriptorPool::Builder &DescriptorPool::Builder::addPoolSize(const VkDescriptorType descriptorType,
                                                              const uint32_t count) {
    poolSizes.push_back({descriptorType, count});
    return *this;
}

DescriptorPool::Builder &DescriptorPool::Builder::setPoolFlags(const VkDescriptorPoolCreateFlags flags) {
    poolFlags = flags;
    return *this;
}

DescriptorPool::Builder &DescriptorPool::Builder::setMaxSets(const uint32_t count) {
    maxSets = count;
    return *this;
}

std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const {
    return std::make_unique<DescriptorPool>(deviceRef, maxSets, poolFlags, poolSizes);
}

DescriptorPool::DescriptorPool(Device &device, const uint32_t maxSets, const VkDescriptorPoolCreateFlags poolFlags,
                               const std::vector<VkDescriptorPoolSize> &poolSizes) : deviceRef{device} {
    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
    descriptorPoolInfo.maxSets = maxSets;
    descriptorPoolInfo.flags = poolFlags;

    if (vkCreateDescriptorPool(deviceRef.device(), &descriptorPoolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

DescriptorPool::~DescriptorPool() { vkDestroyDescriptorPool(deviceRef.device(), m_DescriptorPool, nullptr); }

bool DescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout,
                                        VkDescriptorSet &descriptor) const {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_DescriptorPool;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.descriptorSetCount = 1;

    // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
    // a new pool whenever an old pool fills up. But this is beyond our current scope
    if (vkAllocateDescriptorSets(deviceRef.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
        return false;
    }
    return true;
}

void DescriptorPool::freeDescriptors(const std::vector<VkDescriptorSet> &descriptors) const {
    vkFreeDescriptorSets(deviceRef.device(), m_DescriptorPool, static_cast<uint32_t>(descriptors.size()),
                         descriptors.data());
}

void DescriptorPool::resetPool() const { vkResetDescriptorPool(deviceRef.device(), m_DescriptorPool, 0); }

// *************** Descriptor Writer *********************

DescriptorWriter::DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool) :
    setLayoutRef{setLayout}, poolRef{pool} {}

DescriptorWriter &DescriptorWriter::writeBuffer(const uint32_t binding, const VkDescriptorBufferInfo *bufferInfo) {
    assert(setLayoutRef.m_Bindings.count(binding) == 1 && "Layout does not contain specified binding");

    const auto &bindingDescription = setLayoutRef.m_Bindings[binding];

    assert(bindingDescription.descriptorCount == 1 &&
           "Binding single descriptor info, but binding expects multiple");

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pBufferInfo = bufferInfo;
    write.descriptorCount = 1;

    m_Writes.push_back(write);
    return *this;
}

DescriptorWriter &DescriptorWriter::writeImage(const uint32_t binding, const VkDescriptorImageInfo *imageInfo) {
    assert(setLayoutRef.m_Bindings.count(binding) == 1 && "Layout does not contain specified binding");

    const auto &bindingDescription = setLayoutRef.m_Bindings[binding];

    assert(bindingDescription.descriptorCount == 1 &&
           "Binding single descriptor info, but binding expects multiple");

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pImageInfo = imageInfo;
    write.descriptorCount = 1;

    m_Writes.push_back(write);

    return *this;
}

bool DescriptorWriter::build(VkDescriptorSet &set) {
    if (const bool success = poolRef.allocateDescriptor(setLayoutRef.getDescriptorSetLayout(), set); !success) {
        return false;
    }
    overwrite(set);
    return true;
}

void DescriptorWriter::overwrite(const VkDescriptorSet &set) {
    for (auto &write: m_Writes) {
        write.dstSet = set;
    }
    vkUpdateDescriptorSets(poolRef.deviceRef.device(), static_cast<uint32_t>(m_Writes.size()), m_Writes.data(), 0, nullptr);
}

} // Namespace KaguEngine
