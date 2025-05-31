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
    void centerWindow(GLFWwindow *window, GLFWmonitor *monitor);

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
    const auto monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE); // GLFW_FALSE when GUI

    m_Window = glfwCreateWindow(mode->width / 2, mode->height / 2, m_WindowName.c_str(), nullptr, nullptr);
    centerWindow(m_Window, monitor);
    glfwSetWindowUserPointer(m_Window, this);
    glfwSetFramebufferSizeCallback(m_Window, framebufferResizeCallback);
}

void Window::createWindowSurface(const VkInstance instance, VkSurfaceKHR* surface) const {
    if (glfwCreateWindowSurface(instance, m_Window, nullptr, surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface");
    }
}

void Window::centerWindow(GLFWwindow *window, GLFWmonitor *monitor) {
    if (!monitor)
        return;

    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    if (!mode)
        return;

    int monitorX, monitorY;
    glfwGetMonitorPos(monitor, &monitorX, &monitorY);

    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    glfwSetWindowPos(
        window,
        monitorX + (mode->width - windowWidth) / 2,
        monitorY + (mode->height - windowHeight) / 2);
}

void Window::framebufferResizeCallback(GLFWwindow* window, const int width, const int height) {
    const auto resizedWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
    resizedWindow->m_FramebufferResized = true;
    resizedWindow->m_Width = width;
    resizedWindow->m_Height = height;
}

} // Namespace KaguEngine
