module;

// libs
#include <vulkan/vulkan.h>

export module KaguEngine.System.PointLight;

// std
import std;

import KaguEngine.Device;
import KaguEngine.Entity;
import KaguEngine.FrameInfo;
import KaguEngine.Pipeline;

export namespace KaguEngine {

class PointLightSystem {
public:
    PointLightSystem(Device &device, VkFormat colorFormat, VkFormat depthFormat, VkDescriptorSetLayout globalSetLayout);
    ~PointLightSystem();

    PointLightSystem(const PointLightSystem &) = delete;
    PointLightSystem &operator=(const PointLightSystem &) = delete;

    static void update(const FrameInfo &frameInfo, GlobalUbo &ubo);
    void render(const FrameInfo &frameInfo) const;

private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void createPipeline(VkFormat colorFormat, VkFormat depthFormat);

    Device &m_Device;

    std::unique_ptr<Pipeline> m_Pipeline;
    VkPipelineLayout m_pipelineLayout;
};

} // Namespace KaguEngine