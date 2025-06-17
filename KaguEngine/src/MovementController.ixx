module;

// libs
#include <GLFW/glfw3.h>

export module KaguEngine.MovementController;

// std
import std;

import KaguEngine.Entity;

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
        int fullScreen = GLFW_KEY_F11;
    };

    void moveInPlaneXZ(GLFWwindow *window, float dt, Entity &entity) const;

    KeyMappings keys{};
    float moveSpeed{3.f};
    float lookSpeed{1.5f};
};

} // Namespace KaguEngine