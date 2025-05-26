#pragma once

#include "../Camera.hpp"
#include "../Device.hpp"
#include "../FrameInfo.hpp"
#include "../Entity.hpp"
#include "../Pipeline.hpp"

// std
#include <memory>
#include <vector>
#include <iostream>

namespace KaguEngine {

class PointLightSystem {
public:
    PointLightSystem(
        Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
    ~PointLightSystem();

    PointLightSystem(const PointLightSystem &) = delete;
    PointLightSystem &operator=(const PointLightSystem &) = delete;

    static void update(const FrameInfo &frameInfo, GlobalUbo &ubo);
    void render(const FrameInfo &frameInfo) const;

private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void createPipeline(VkRenderPass renderPass);

    Device &m_Device;

    std::unique_ptr<Pipeline> m_Pipeline;
    VkPipelineLayout m_pipelineLayout;
};

} // Namespace KaguEngine