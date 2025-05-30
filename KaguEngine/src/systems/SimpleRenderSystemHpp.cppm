module;

// libs
#include <vulkan/vulkan.h>

// std
#include <memory>

export module SimpleRenderSystem:Hpp;

export import Camera;
export import Device;
export import Entity;
export import FrameInfo;
export import Pipeline;

export namespace KaguEngine {

class SimpleRenderSystem {
public:
    SimpleRenderSystem(Device &device, VkRenderPass renderPass,
                   VkDescriptorSetLayout globalSetLayout,
                   VkDescriptorSetLayout materialSetLayout);
    ~SimpleRenderSystem();

    SimpleRenderSystem(const SimpleRenderSystem &) = delete;
    SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

    void renderGameObjects(const FrameInfo &frameInfo) const;

private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout materialSetLayout);
    void createPipeline(VkRenderPass renderPass);

    Device &m_Device;

    std::unique_ptr<Pipeline> m_Pipeline;
    VkPipelineLayout m_pipelineLayout;
};

} // Namespace KaguEngine
