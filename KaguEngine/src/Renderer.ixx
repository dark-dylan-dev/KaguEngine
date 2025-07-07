module;

// libs
#include <vulkan/vulkan.h>
#include <glm/vec4.hpp>

// std
#include <cassert>

export module KaguEngine.Renderer;

// std
import std;

import KaguEngine.Device;
import KaguEngine.SwapChain;
import KaguEngine.Window;

export namespace KaguEngine {

class Renderer {
public:
    Renderer(Window &window, Device &device);
    ~Renderer();

    Renderer(const Renderer &) = delete;
    Renderer &operator=(const Renderer &) = delete;

    [[nodiscard]] float getAspectRatio() const { return m_SwapChain->extentAspectRatio(); }
    [[nodiscard]] bool isFrameInProgress() const { return m_isFrameStarted; }

    [[nodiscard]] VkCommandBuffer getCurrentCommandBuffer() const {
        assert(m_isFrameStarted &&"Cannot get command buffer when frame not in progress");
        return m_commandBuffers[m_currentFrameIndex];
    }

    [[nodiscard]] int getFrameIndex() const {
        assert(m_isFrameStarted && "Cannot get frame index when frame not in progress");
        return m_currentFrameIndex;
    }

    VkCommandBuffer beginFrame();
    void endFrame();
    void beginSwapChainRendering(VkCommandBuffer commandBuffer) const;
    void endSwapChainRendering(VkCommandBuffer commandBuffer) const;

    // Off screen render pass
    void beginOffscreenRendering(VkCommandBuffer commandBuffer);
    void endOffscreenRendering(VkCommandBuffer commandBuffer) const;
    void transitionOffscreenImageForImGui(VkCommandBuffer commandBuffer);

    [[nodiscard]] VkDescriptorSet getOffscreenImGuiDescriptorSet() const { return m_offscreenImGuiDescriptorSet; }
    [[nodiscard]] VkExtent2D getOffscreenExtent() const { return m_offscreenExtent; }
    [[nodiscard]] VkFormat getOffscreenFormat() const { return m_offscreenFormat; }
    [[nodiscard]] VkFormat getOffscreenDepthFormat() const { return m_offscreenDepthFormat; }

    std::unique_ptr<SwapChain>& getSwapChain() { return m_SwapChain; }
    glm::vec4 clearColor = { 0.1f, 0.1f, 0.15f, 1.0f };

private:
    void createCommandBuffers();
    void freeCommandBuffers();
    void recreateSwapChain();

    Window &windowRef;
    Device &deviceRef;
    std::unique_ptr<SwapChain> m_SwapChain;
    std::vector<VkCommandBuffer> m_commandBuffers;

    uint32_t m_currentImageIndex;
    int m_currentFrameIndex{0};
    bool m_isFrameStarted{false};

    VkDescriptorSet m_offscreenImGuiDescriptorSet = VK_NULL_HANDLE;
    VkDescriptorPool m_offscreenDescriptorPool = VK_NULL_HANDLE;
    VkImageLayout m_offscreenCurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkDescriptorSetLayout m_offscreenDescriptorSetLayout = VK_NULL_HANDLE;
    VkFormat m_offscreenFormat = VK_FORMAT_B8G8R8A8_UNORM;
    VkFormat m_offscreenDepthFormat;
    VkExtent2D m_offscreenExtent{};
    VkSampler m_offscreenSampler = VK_NULL_HANDLE;

    // Multi sampled color image
    VkImage m_offscreenImage = VK_NULL_HANDLE;
    VkDeviceMemory m_offscreenImageMemory = VK_NULL_HANDLE;
    VkImageView m_offscreenImageView = VK_NULL_HANDLE;
    // Resolve image - Not multi sampled
    VkImage m_offscreenResolveImage = VK_NULL_HANDLE;
    VkDeviceMemory m_offscreenResolveMemory = VK_NULL_HANDLE;
    VkImageView m_offscreenResolveImageView = VK_NULL_HANDLE;

    // Depth attachment
    VkImage m_offscreenDepthImage = VK_NULL_HANDLE;
    VkDeviceMemory m_offscreenDepthMemory = VK_NULL_HANDLE;
    VkImageView m_offscreenDepthView = VK_NULL_HANDLE;

    void createOffscreenResources();
    void cleanupOffscreenResources();
    void createOffscreenDescriptorSet();
};

} // Namespace KaguEngine