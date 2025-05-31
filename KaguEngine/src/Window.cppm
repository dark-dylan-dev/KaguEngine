module;

// libs
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// std
import std;

export module KaguEngine.Window;

export namespace KaguEngine {

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

// .cpp part

export namespace KaguEngine {

    Window::Window(const int w, const int h, std::string name) : m_Width{w}, m_Height{h}, m_WindowName{std::move(name)} {
        initWindow();
    }

    Window::~Window() {
        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }

    void Window::initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        m_Window = glfwCreateWindow(m_Width, m_Height, m_WindowName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(m_Window, this);
        glfwSetFramebufferSizeCallback(m_Window, framebufferResizeCallback);
    }

    void Window::createWindowSurface(const VkInstance instance, VkSurfaceKHR* surface) const {
        if (glfwCreateWindowSurface(instance, m_Window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface");
        }
    }

    void Window::framebufferResizeCallback(GLFWwindow* window, const int width, const int height) {
        const auto resizedWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
        resizedWindow->m_FramebufferResized = true;
        resizedWindow->m_Width = width;
        resizedWindow->m_Height = height;
    }

} // Namespace KaguEngine
