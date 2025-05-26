#pragma once

#include "Window.hpp"

// std lib headers
#include <string>
#include <vector>

namespace KaguEngine {

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool graphicsFamilyHasValue = false;
    bool presentFamilyHasValue = false;
    [[nodiscard]] bool isComplete() const { return graphicsFamilyHasValue && presentFamilyHasValue; }
};

class Device {
public:
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    explicit Device(Window &window);
    ~Device();

    // Not copyable or movable
    Device(const Device &) = delete;
    Device &operator=(const Device &) = delete;
    Device(Device &&) = delete;
    Device &operator=(Device &&) = delete;

    [[nodiscard]] VkCommandPool getCommandPool() const { return commandPool; }
    [[nodiscard]] VkDevice device() const { return m_Device; }
    [[nodiscard]] VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
    [[nodiscard]] VkSurfaceKHR surface() const { return m_Surface; }
    [[nodiscard]] VkQueue graphicsQueue() const { return m_GraphicsQueue; }
    [[nodiscard]] VkQueue presentQueue() const { return m_PresentQueue; }

    [[nodiscard]] SwapChainSupportDetails getSwapChainSupport() const { return querySwapChainSupport(physicalDevice); }
    [[nodiscard]] uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
    [[nodiscard]] QueueFamilyIndices findPhysicalQueueFamilies() const { return findQueueFamilies(physicalDevice); }
    [[nodiscard]] VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                 VkFormatFeatureFlags features) const;
    [[nodiscard]] const VkSampleCountFlagBits& getSampleCount() const { return msaaSamples; }

    // Buffer Helper Functions
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                      VkBuffer &buffer, VkDeviceMemory &bufferMemory) const;
    [[nodiscard]] VkCommandBuffer beginSingleTimeCommands() const;
    void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height,
                           uint32_t layerCount) const;

    void createImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties, VkImage &image,
                             VkDeviceMemory &imageMemory) const;

    VkPhysicalDeviceProperties properties;

private:
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();

    // helper functions
    bool isDeviceSuitable(VkPhysicalDevice device) const;
    [[nodiscard]] std::vector<const char *> getRequiredExtensions() const;
    [[nodiscard]] bool checkValidationLayerSupport() const;
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;
    VkSampleCountFlagBits getMaxUsableSampleCount() const;

    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    void hasGflwRequiredInstanceExtensions() const;
    bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    Window &windowRef;
    VkCommandPool commandPool;

    VkDevice m_Device;
    VkSurfaceKHR m_Surface;
    VkQueue m_GraphicsQueue;
    VkQueue m_PresentQueue;
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

    const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};

} // Namespace KaguEngine
