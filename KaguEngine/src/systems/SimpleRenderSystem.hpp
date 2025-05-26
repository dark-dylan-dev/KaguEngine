#pragma once

#include "../Camera.hpp"
#include "../Device.hpp"
#include "../Entity.hpp"
#include "../FrameInfo.hpp"
#include "../Pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace KaguEngine {

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
