module;

// libs
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

module KaguEngine.Entity;

// std
import std;

import KaguEngine.Model;
import KaguEngine.Texture;

namespace KaguEngine {

glm::mat4 TransformComponent::mat4() const {
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    return glm::mat4{
        {
             scale.x * (c1 * c3 + s1 * s2 * s3),
             scale.x * (c2 * s3),
             scale.x * (c1 * s2 * s3 - c3 * s1),
             0.0f,
        },
        {
             scale.y * (c3 * s1 * s2 - c1 * s3),
             scale.y * (c2 * c3),
             scale.y * (c1 * c3 * s2 + s1 * s3),
             0.0f,
        },
        {
             scale.z * (c2 * s1),
             scale.z * (-s2),
             scale.z * (c1 * c2),
             0.0f,
        },
        {translation.x, translation.y, translation.z, 1.0f}
    };
}

glm::mat3 TransformComponent::normalMatrix() const {
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    const glm::vec3 invScale = 1.0f / scale;

    return glm::mat3{
        {
            invScale.x * (c1 * c3 + s1 * s2 * s3),
            invScale.x * (c2 * s3),
            invScale.x * (c1 * s2 * s3 - c3 * s1),
        },
        {
            invScale.y * (c3 * s1 * s2 - c1 * s3),
            invScale.y * (c2 * c3),
            invScale.y * (c1 * c3 * s2 + s1 * s3),
        },
        {
            invScale.z * (c2 * s1),
            invScale.z * (-s2),
            invScale.z * (c1 * c2),
        },
    };
}

Entity Entity::makePointLight(const float intensity, const float radius, const glm::vec3 color) {
    Entity lightEntity = createEntity();
    lightEntity.color = color;
    lightEntity.transform.scale.x = radius;
    lightEntity.pointLight = std::make_unique<PointLightComponent>();
    lightEntity.pointLight->lightIntensity = intensity;
    return lightEntity;
}

} // Namespace KaguEngine