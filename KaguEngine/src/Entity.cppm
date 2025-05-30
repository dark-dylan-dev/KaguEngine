module;

// libs
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// std
import std;

import Model;
import Texture;

export module Entity;

export namespace KaguEngine {

struct TransformComponent {
    glm::vec3 translation{};
    glm::vec3 scale{1.f, 1.f, 1.f};
    glm::vec3 rotation{};

    // Matrix corresponds to Translate * Ry * Rx * Rz * Scale
    // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
    // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
    [[nodiscard]] glm::mat4 mat4() const;
    [[nodiscard]] glm::mat3 normalMatrix() const;
};

struct PointLightComponent {
    float lightIntensity = 1.0f;
};

class Entity {
public:
    using id_t = unsigned int;
    using Map = std::unordered_map<id_t, Entity>;

    static Entity createEntity() {
        static id_t currentId = 0;
        return Entity{currentId++};
    }

    static Entity makePointLight(float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

    // Non copyable
    Entity(const Entity &) = delete;
    Entity &operator=(const Entity &) = delete;
    Entity(Entity &&) = default;
    Entity &operator=(Entity &&) = default;

    [[nodiscard]] id_t getId() const { return m_Id; }

    glm::vec3 color{};
    TransformComponent transform{};

    // Optional pointer components
    std::unique_ptr<Texture> texture = nullptr;
    std::shared_ptr<Model> model{};
    Texture::Material material{};
    std::unique_ptr<PointLightComponent> pointLight = nullptr;

private:
    explicit Entity(const id_t objId) : m_Id{objId} {}

    id_t m_Id;
};

} // Namespace KaguEngine

// .cpp part

export namespace KaguEngine {

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
    return lightEntity; // rvalue
}

} // Namespace KaguEngine
