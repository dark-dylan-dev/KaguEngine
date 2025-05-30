module;

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

// std
import std;
import std.compat;

export module SimpleRenderSystem;

import Camera;
import Device;
import Entity;
import FrameInfo;
import Pipeline;

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

// .cpp part

export namespace KaguEngine {

struct SimplePushConstantData {
    glm::mat4 modelMatrix{1.f};
    glm::mat4 normalMatrix{1.f};
};

SimpleRenderSystem::SimpleRenderSystem(
    Device &device,
    const VkRenderPass renderPass,
    const VkDescriptorSetLayout globalSetLayout,
    const VkDescriptorSetLayout materialSetLayout
) : m_Device{device}
{
    createPipelineLayout(globalSetLayout, materialSetLayout);
    createPipeline(renderPass);
}

SimpleRenderSystem::~SimpleRenderSystem() {
    vkDestroyPipelineLayout(m_Device.device(), m_pipelineLayout, nullptr);
}

void SimpleRenderSystem::createPipelineLayout(const VkDescriptorSetLayout globalSetLayout,
                                              const VkDescriptorSetLayout materialSetLayout) {
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

    const std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
        globalSetLayout,      // set = 0
        materialSetLayout     // set = 1
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(m_Device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }
}

void SimpleRenderSystem::createPipeline(const VkRenderPass renderPass) {
    assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = m_pipelineLayout;
    pipelineConfig.multisampleInfo.rasterizationSamples = m_Device.getSampleCount();
    m_Pipeline = std::make_unique<Pipeline>(
        m_Device,
        "assets/shaders/simple_shader.vert.spv",
        "assets/shaders/simple_shader.frag.spv",
        pipelineConfig);
}

void SimpleRenderSystem::renderGameObjects(const FrameInfo &frameInfo) const {
    m_Pipeline->bind(frameInfo.commandBuffer);

    // Bind global descriptor set (set = 0)
    vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

    for (auto &val: frameInfo.sceneEntitiesRef | std::views::values) {
        auto &obj = val;
        if (!obj.model) continue;

        // Only binds material set if it exists
        if (obj.material.descriptorSet != VK_NULL_HANDLE) {
            vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    m_pipelineLayout, 1, 1, &obj.material.descriptorSet, 0, nullptr);
        } else {
            // Handles no material component
        }

        SimplePushConstantData push{};
        push.modelMatrix = obj.transform.mat4();
        push.normalMatrix = obj.transform.normalMatrix();

        vkCmdPushConstants(frameInfo.commandBuffer, m_pipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                           sizeof(SimplePushConstantData), &push);

        obj.model->bind(frameInfo.commandBuffer);
        obj.model->draw(frameInfo.commandBuffer);
    }
}

} // Namespace KaguEngine