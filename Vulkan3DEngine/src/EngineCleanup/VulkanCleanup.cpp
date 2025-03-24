#include "../Vulkan3DEngine.hpp"

void Vulkan3DEngine::cleanup() {
    // Destroying the debugger
    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    // Destroying the command pool
    vkDestroyCommandPool(device, commandPool, nullptr);

    // Destroying the framebuffers
    for (auto framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    // Destroying the semaphores
    vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
    vkDestroyFence(device, inFlightFence, nullptr);

    // Destroying the swap chain
    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(device, swapChain, nullptr);

    // Destroying the pipeline layout and the render pass
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    // Destroying the device
    vkDestroyDevice(device, nullptr);

    // Destroying the window surface
    vkDestroySurfaceKHR(instance, surface, nullptr);

    // Destroying the Vulkan instance
    vkDestroyInstance(instance, nullptr);

    // Destroying the window and terminating the program
    glfwDestroyWindow(window);
    glfwTerminate();
}