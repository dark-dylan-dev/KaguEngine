module;

// libs
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <cmath>
#include <limits>

export module MovementController;

import Entity;

export namespace KaguEngine {

class KeyboardMovementController {
public:
    struct KeyMappings {
        int moveLeft = GLFW_KEY_A;
        int moveRight = GLFW_KEY_D;
        int moveForward = GLFW_KEY_W;
        int moveBackward = GLFW_KEY_S;
        int moveUp = GLFW_KEY_E;
        int moveDown = GLFW_KEY_Q;
        int lookLeft = GLFW_KEY_LEFT;
        int lookRight = GLFW_KEY_RIGHT;
        int lookUp = GLFW_KEY_UP;
        int lookDown = GLFW_KEY_DOWN;
    };

    void moveInPlaneXZ(GLFWwindow *window, float dt, Entity &entity) const;

    KeyMappings keys{};
    float moveSpeed{3.f};
    float lookSpeed{1.5f};
};

} // Namespace KaguEngine

// .cpp part

export namespace KaguEngine {

void KeyboardMovementController::moveInPlaneXZ(GLFWwindow* window, const float dt, Entity& entity) const {
    glm::vec3 rotate{0};
    if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS)
        rotate.y += 1.f;
    if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS)
        rotate.y -= 1.f;
    if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS)
        rotate.x += 1.f;
    if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS)
        rotate.x -= 1.f;

    if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
        entity.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
    }

    // limit pitch values between about +/- 85ish degrees
    entity.transform.rotation.x = glm::clamp(entity.transform.rotation.x, -1.5f, 1.5f);
    entity.transform.rotation.y = glm::mod(entity.transform.rotation.y, glm::two_pi<float>());

    float yaw = entity.transform.rotation.y;
    const glm::vec3 forwardDir{std::sin(yaw), 0.f, std::cos(yaw)};
    const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
    constexpr glm::vec3 upDir{0.f, -1.f, 0.f};

    glm::vec3 moveDir{0.f};
    if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS)
        moveDir += forwardDir;
    if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS)
        moveDir -= forwardDir;
    if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS)
        moveDir += rightDir;
    if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS)
        moveDir -= rightDir;
    if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS)
        moveDir += upDir;
    if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS)
        moveDir -= upDir;

    if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
        entity.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
    }
}

} // Namespace KaguEngine
