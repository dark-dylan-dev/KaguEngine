module;

// libs
#include <glm/glm.hpp>

export module KaguEngine.Camera;

export namespace KaguEngine {

class Camera {
public:
    void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
    void setPerspectiveProjection(float fovy, float aspect, float near, float far);

    void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
    void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
    void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

    [[nodiscard]] const glm::mat4 &getProjection() const { return projectionMatrix; }
    [[nodiscard]] const glm::mat4 &getView() const { return viewMatrix; }
    [[nodiscard]] const glm::mat4 &getInverseView() const { return inverseViewMatrix; }
    [[nodiscard]] glm::vec3 getPosition() const { return {inverseViewMatrix[3]}; }

private:
    glm::mat4 projectionMatrix{1.f};
    glm::mat4 viewMatrix{1.f};
    glm::mat4 inverseViewMatrix{1.f};
};

} // Namespace KaguEngine
