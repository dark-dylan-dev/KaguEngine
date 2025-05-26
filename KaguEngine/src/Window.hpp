#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace KaguEngine {

class Window {
public:
    Window(int w, int h, const std::string &name);
    ~Window();

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    bool shouldClose() const { return glfwWindowShouldClose(m_Window); }
    VkExtent2D getExtent() const { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
    bool wasWindowResized() const { return framebufferResized; }
    void resetWindowResizedFlag() { framebufferResized = false; }
    GLFWwindow *getGLFWwindow() const { return m_Window; }

    void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) const;

private:
    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
    void initWindow();

    int width;
    int height;
    bool framebufferResized = false;

    std::string windowName;
    GLFWwindow* m_Window;
};

} // Namespace KaguEngine
