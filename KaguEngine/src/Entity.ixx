module;

// libs
#include <glm/glm.hpp>

export module KaguEngine.Entity;

// std
import std;

import KaguEngine.Model;
import KaguEngine.Texture;

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
    Entity(Entity &&other) noexcept {
        name       = other.name;
        m_Id       = other.m_Id;
        color      = other.color;
        transform  = other.transform;
        texture    = std::move(other.texture);
        model      = std::move(other.model);
        material   = other.material;
        pointLight = std::move(other.pointLight);
    }
    Entity &operator=(Entity &&) = default;

    [[nodiscard]] id_t getId() const { return m_Id; }

    std::string name;
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