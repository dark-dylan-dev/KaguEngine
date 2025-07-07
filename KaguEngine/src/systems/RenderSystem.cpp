module;

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

module KaguEngine.System.Render;

// std
import std;

import KaguEngine.Camera;
import KaguEngine.Device;
import KaguEngine.Entity;
import KaguEngine.FrameInfo;
import KaguEngine.Pipeline;

namespace KaguEngine {

struct SimplePushConstantData {
    glm::mat4 modelMatrix{1.f};
    glm::vec3 modelColor{1.f};
    float modelAlpha{1.f};
};

RenderSystem::RenderSystem(
    Device &device,
    const VkFormat colorFormat,
    const VkFormat depthFormat,
    const VkDescriptorSetLayout globalSetLayout,
    const VkDescriptorSetLayout materialSetLayout
) : m_Device{device}
{
    createPipelineNoTexturesLayout(globalSetLayout);
    createPipelineTexturesLayout(globalSetLayout, materialSetLayout);
    createPipeline(colorFormat, depthFormat);
}

RenderSystem::~RenderSystem() {
    vkDestroyPipelineLayout(m_Device.device(), m_pipelineTexturesLayout, nullptr);
    vkDestroyPipelineLayout(m_Device.device(), m_pipelineNoTexturesLayout, nullptr);
}

void RenderSystem::createPipelineNoTexturesLayout(const VkDescriptorSetLayout globalSetLayout) {
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

    const std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
        globalSetLayout,      // set = 0
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(m_Device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineNoTexturesLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }
}

void RenderSystem::createPipelineTexturesLayout(const VkDescriptorSetLayout globalSetLayout,
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

    if (vkCreatePipelineLayout(m_Device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineTexturesLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }
}

void RenderSystem::createPipeline(const VkFormat colorFormat, const VkFormat depthFormat) {
    assert(m_pipelineTexturesLayout != nullptr && "Cannot create pipeline before pipeline layout");
    assert(m_pipelineNoTexturesLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfigTextures{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfigTextures, true);
    Pipeline::enableAlphaBlending(pipelineConfigTextures);
    Pipeline::enableMSAA(pipelineConfigTextures, m_Device.getSampleCount());
    // ---
    PipelineConfigInfo pipelineConfigNoTextures{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfigNoTextures, false);
    Pipeline::enableAlphaBlending(pipelineConfigNoTextures);
    Pipeline::enableMSAA(pipelineConfigNoTextures, m_Device.getSampleCount());

    pipelineConfigTextures.pipelineLayout = m_pipelineTexturesLayout;
    pipelineConfigTextures.colorAttachmentFormat = colorFormat;
    pipelineConfigTextures.depthAttachmentFormat = depthFormat;
    // ---
    pipelineConfigNoTextures.pipelineLayout = m_pipelineNoTexturesLayout;
    pipelineConfigNoTextures.colorAttachmentFormat = colorFormat;
    pipelineConfigNoTextures.depthAttachmentFormat = depthFormat;

    m_PipelineTextures = std::make_unique<Pipeline>(
        m_Device,
        "assets/shaders/with_textures.vert.spv",
        "assets/shaders/with_textures.frag.spv",
        pipelineConfigTextures);
    m_PipelineNoTextures = std::make_unique<Pipeline>(
        m_Device,
        "assets/shaders/without_textures.vert.spv",
        "assets/shaders/without_textures.frag.spv",
        pipelineConfigNoTextures
    );
}

void RenderSystem::renderGameObjects(const FrameInfo &frameInfo) const {
    for (auto &[id, entity]: frameInfo.sceneEntitiesRef) {
        if (!entity.model) continue;

        SimplePushConstantData push{};
        push.modelMatrix  = entity.transform.mat4();
        push.modelColor   = entity.color;
        push.modelAlpha   = entity.transform.alpha;

        // With textures
        if (entity.texture != nullptr) {
            m_PipelineTextures->bind(frameInfo.commandBuffer);
            vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    m_pipelineTexturesLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);
            vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    m_pipelineTexturesLayout, 1, 1, &entity.material.descriptorSet, 0, nullptr);
            vkCmdPushConstants(frameInfo.commandBuffer, m_pipelineTexturesLayout,
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                               sizeof(SimplePushConstantData), &push);
        }
        // Without textures
        else {
            m_PipelineNoTextures->bind(frameInfo.commandBuffer);
            vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    m_pipelineNoTexturesLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);
            vkCmdPushConstants(frameInfo.commandBuffer, m_pipelineNoTexturesLayout,
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                               sizeof(SimplePushConstantData), &push);
        }

        entity.model->bind(frameInfo.commandBuffer);
        entity.model->draw(frameInfo.commandBuffer);
    }
}

} // Namespace KaguEngine