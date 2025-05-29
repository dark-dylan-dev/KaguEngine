#pragma once

#include "Model.hpp"
#include "Texture.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>

namespace KaguEngine {

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

    // Non copyable, movable
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
