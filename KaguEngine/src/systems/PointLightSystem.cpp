#include "PointLightSystem.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <map>
#include <ranges>
#include <stdexcept>
#include <vector>

namespace KaguEngine {

struct PointLightPushConstants {
    glm::vec4 position{};
    glm::vec4 color{};
    float radius;
};

PointLightSystem::PointLightSystem(Device &device, const VkRenderPass renderPass,
                                   const VkDescriptorSetLayout globalSetLayout) : m_Device{device} {
    createPipelineLayout(globalSetLayout);
    createPipeline(renderPass);
}

PointLightSystem::~PointLightSystem() {
    vkDestroyPipelineLayout(m_Device.device(), m_pipelineLayout, nullptr);
}

void PointLightSystem::createPipelineLayout(const VkDescriptorSetLayout globalSetLayout) {
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PointLightPushConstants);

    const std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if (vkCreatePipelineLayout(m_Device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void PointLightSystem::createPipeline(const VkRenderPass renderPass) {
    assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    Pipeline::enableAlphaBlending(pipelineConfig);
    pipelineConfig.attributeDescriptions.clear();
    pipelineConfig.bindingDescriptions.clear();
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = m_pipelineLayout;
    pipelineConfig.multisampleInfo.rasterizationSamples = m_Device.getSampleCount();
    m_Pipeline = std::make_unique<Pipeline>(m_Device,
        "assets/shaders/point_light.vert.spv",
        "assets/shaders/point_light.frag.spv", pipelineConfig);
}

void PointLightSystem::update(const FrameInfo &frameInfo, GlobalUbo &ubo) {
    const auto rotateLight = glm::rotate(glm::mat4(1.f), 0.5f * frameInfo.frameTime, {0.f, -1.f, 0.f});
    int lightIndex = 0;
    for (auto &val: frameInfo.sceneEntitiesRef | std::views::values) {
        auto &obj = val;
        if (obj.pointLight == nullptr)
            continue;

        assert(lightIndex < MAX_LIGHTS && "Point lights exceed maximum specified");

        // update light position
        obj.transform.translation = glm::vec3(rotateLight * glm::vec4(obj.transform.translation, 1.f));

        // copy light to ubo
        ubo.pointLights[lightIndex].position = glm::vec4(obj.transform.translation, 1.f);
        ubo.pointLights[lightIndex].color = glm::vec4(obj.color, obj.pointLight->lightIntensity);

        lightIndex++;
    }
    ubo.numLights = lightIndex;
}

void PointLightSystem::render(const FrameInfo &frameInfo) const {
    // sort lights
    std::map<float, Entity::id_t> sorted;
    for (const auto &val: frameInfo.sceneEntitiesRef | std::views::values) {
        auto &obj = val;
        if (obj.pointLight == nullptr)
            continue;

        // calculate distance
        auto offset = frameInfo.cameraRef.getPosition() - obj.transform.translation;
        float disSquared = glm::dot(offset, offset);
        sorted[disSquared] = obj.getId();
    }

    m_Pipeline->bind(frameInfo.commandBuffer);

    vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1,
                            &frameInfo.globalDescriptorSet, 0, nullptr);

    // iterate through sorted lights in reverse order (furthest -> nearest)
    for (auto &[first, second] : std::ranges::reverse_view(sorted)) {
        // use scene entities reference id to find light object
        auto &obj = frameInfo.sceneEntitiesRef.at(second);

        PointLightPushConstants push{};
        push.position = glm::vec4(obj.transform.translation, 1.f);
        push.color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
        push.radius = obj.transform.scale.x;

        vkCmdPushConstants(frameInfo.commandBuffer, m_pipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                           sizeof(PointLightPushConstants), &push);
        vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
    }
}

} // Namespace KaguEngine
