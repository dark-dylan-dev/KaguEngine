#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace KaguEngine {

class Window {
public:
    Window(int w, int h, std::string name);
    ~Window();

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    [[nodiscard]] bool shouldClose()          const { return glfwWindowShouldClose(m_Window); }
    [[nodiscard]] bool windowResized()        const { return m_FramebufferResized; }
    [[nodiscard]] GLFWwindow *getGLFWwindow() const { return m_Window; }
    [[nodiscard]] VkExtent2D getExtent()      const {
        return {static_cast<uint32_t>(m_Width), static_cast<uint32_t>(m_Height)};
    }

    void resetWindowResizedFlag() { m_FramebufferResized = false; }

    void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) const;

private:
    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
    void initWindow();

    int m_Width;
    int m_Height;
    bool m_FramebufferResized = false;

    std::string m_WindowName;
    GLFWwindow* m_Window;
};

} // Namespace KaguEngine
