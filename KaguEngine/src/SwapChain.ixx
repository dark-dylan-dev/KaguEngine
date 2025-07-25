module;

// libs
#include <vulkan/vulkan.h>

export module KaguEngine.SwapChain;

// std
import std;

import KaguEngine.Device;

export namespace KaguEngine {

class SwapChain {
public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    SwapChain(Device &deviceRef, VkExtent2D windowExtent);
    SwapChain(Device &deviceRef, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous);

    ~SwapChain();

    SwapChain(const SwapChain &) = delete;
    SwapChain &operator=(const SwapChain &) = delete;

    [[nodiscard]] VkImageView getImageView(const int index) const     { return m_SwapChainImageViews[index]; }
    [[nodiscard]] VkImage getImage(const int index) const             { return m_SwapChainImages[index]; }
    [[nodiscard]] VkImageView getDepthImageView(const int index) const { return m_DepthImageViews[index]; }
    [[nodiscard]] VkImageView getMultisampleColorImageView(const int index) const { return m_MultisampleColorImageViews[index]; }
    [[nodiscard]] size_t imageCount() const                           { return m_SwapChainImages.size(); }
    [[nodiscard]] VkFormat* getSwapChainImageFormat()                 { return &m_SwapChainImageFormat; }
    [[nodiscard]] VkExtent2D getSwapChainExtent() const               { return m_SwapChainExtent; }
    [[nodiscard]] uint32_t width() const                              { return m_SwapChainExtent.width; }
    [[nodiscard]] uint32_t height() const                             { return m_SwapChainExtent.height; }

    [[nodiscard]] float extentAspectRatio() const {
        return static_cast<float>(m_SwapChainExtent.width) / static_cast<float>(m_SwapChainExtent.height);
    }
    [[nodiscard]] VkFormat findDepthFormat() const;

    VkResult acquireNextImage(uint32_t *imageIndex) const;
    VkResult submitCommandBuffers(const VkCommandBuffer *buffers, const uint32_t *imageIndex);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) const;
    [[nodiscard]] bool compareSwapFormats(const SwapChain &swapChain) const {
        return swapChain.m_SwapChainDepthFormat == m_SwapChainDepthFormat &&
               swapChain.m_SwapChainImageFormat == m_SwapChainImageFormat;
    }

private:
    void init();
    void createSwapChain();
    void createImageViews();
    void createDepthResources();
    void createColorResources();
    void createSyncObjects();

    // Helper functions
    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

    static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    [[nodiscard]] VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const;

    VkFormat m_SwapChainImageFormat;
    VkFormat m_SwapChainDepthFormat;
    VkExtent2D m_SwapChainExtent;

    std::vector<VkImage> m_DepthImages;
    std::vector<VkDeviceMemory> m_DepthImageMemories;
    std::vector<VkImageView> m_DepthImageViews;
    std::vector<VkImage> m_SwapChainImages;
    std::vector<VkImageView> m_SwapChainImageViews;
    std::vector<VkImage> m_MultisampleColorImages;
    std::vector<VkDeviceMemory> m_MultisampleColorImageMemories;
    std::vector<VkImageView> m_MultisampleColorImageViews;

    Device &deviceRef;
    VkExtent2D m_WindowExtent;

    VkSwapchainKHR m_SwapChain;
    std::shared_ptr<SwapChain> m_OldSwapChain;

    std::vector<VkSemaphore> m_ImageAvailableSemaphores;
    std::vector<VkSemaphore> m_RenderFinishedSemaphores;
    std::vector<VkFence> m_InFlightFences;
    std::vector<VkFence> m_ImagesInFlight;
    size_t m_CurrentFrame = 0;
};

} // Namespace KaguEngine