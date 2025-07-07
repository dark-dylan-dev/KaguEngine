module;

// libs
#include <vulkan/vulkan.h>

export module KaguEngine.System.Render;

// std
import std;

import KaguEngine.Camera;
import KaguEngine.Device;
import KaguEngine.Entity;
import KaguEngine.FrameInfo;
import KaguEngine.Pipeline;

export namespace KaguEngine {

class RenderSystem {
public:
    RenderSystem(Device &device, VkFormat colorFormat, VkFormat depthFormat,
                   VkDescriptorSetLayout globalSetLayout,
                   VkDescriptorSetLayout materialSetLayout);
    ~RenderSystem();

    RenderSystem(const RenderSystem &) = delete;
    RenderSystem &operator=(const RenderSystem &) = delete;

    void renderGameObjects(const FrameInfo &frameInfo) const;

private:
    void createPipelineTexturesLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout materialSetLayout);
    void createPipelineNoTexturesLayout(VkDescriptorSetLayout globalSetLayout);
    void createPipeline(VkFormat colorFormat, VkFormat depthFormat);

    Device &m_Device;

    std::unique_ptr<Pipeline> m_PipelineNoTextures;
    std::unique_ptr<Pipeline> m_PipelineTextures;
    VkPipelineLayout m_pipelineTexturesLayout;
    VkPipelineLayout m_pipelineNoTexturesLayout;
};

} // Namespace KaguEngine