#include "Window.hpp"

// std
#include <stdexcept>

namespace KaguEngine {

Window::Window(const int w, const int h, const std::string &name) : width{w}, height{h}, windowName{name} {
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

    m_Window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
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
    resizedWindow->framebufferResized = true;
    resizedWindow->width = width;
    resizedWindow->height = height;
}

} // Namespace KaguEngine
