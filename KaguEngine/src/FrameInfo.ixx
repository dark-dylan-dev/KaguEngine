module;

// libs
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

import KaguEngine.Camera;
import KaguEngine.Entity;

export module KaguEngine.FrameInfo;

export namespace KaguEngine {

constexpr int MAX_LIGHTS = 10;

struct PointLight {
    glm::vec4 position{}; // ignore w
    glm::vec4 color{}; // w is intensity
};

struct GlobalUbo {
    glm::mat4 projection{1.f};
    glm::mat4 view{1.f};
    glm::mat4 inverseView{1.f};
    glm::vec4 ambientLightColor{1.f, 1.f, 1.f, 0.02f}; // w is intensity
    PointLight pointLights[MAX_LIGHTS];
    int numLights;
};

struct FrameInfo {
    int frameIndex;
    float frameTime;
    VkCommandBuffer commandBuffer;
    Camera &cameraRef;
    VkDescriptorSet globalDescriptorSet;
    Entity::Map &sceneEntitiesRef;
};

} // Namespace KaguEngine
