module;

// libs
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// std
#include <stdexcept>
#include <utility>

export module Window;
export import :Hpp;

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
