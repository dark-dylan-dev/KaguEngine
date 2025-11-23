module;

// libs
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

module KaguEngine.MovementController;

// std
import std;

import KaguEngine.Entity;

namespace {
    int width, height, xpos, ypos;
    bool isFullscreen = false;
    bool isHolding = false;
}

namespace KaguEngine {

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

    // Fullscreen check
    if (glfwGetKey(window, keys.fullScreen) == GLFW_PRESS && !isHolding) {
        if (!isFullscreen) {
            glfwGetWindowSize(window, &width, &height);
            glfwGetWindowPos(window, &xpos, &ypos);
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
        }
        else {
            glfwSetWindowMonitor(window, nullptr, xpos, ypos, width, height, 0);
        }
        isFullscreen = !isFullscreen;
        isHolding = true;
    }
    else if (glfwGetKey(window, keys.fullScreen) == GLFW_RELEASE && isHolding) {
        isHolding = false;
    }

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