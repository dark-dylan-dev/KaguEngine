module;

// libs
#include <vulkan/vulkan.h>

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

    [[nodiscard]] VkRenderPass getSwapChainRenderPass() const { return m_SwapChain->getRenderPass(); }
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
    void beginSwapChainRenderPass(VkCommandBuffer commandBuffer) const;
    void endSwapChainRenderPass(VkCommandBuffer commandBuffer) const;

    // Off screen render pass
    void beginOffscreenRenderPass(VkCommandBuffer commandBuffer);
    void endOffscreenRenderPass(VkCommandBuffer commandBuffer) const;
    void transitionOffscreenImageForImGui();

    [[nodiscard]] VkDescriptorSet getOffscreenImGuiDescriptorSet() const { return m_offscreenImGuiDescriptorSet; }
    [[nodiscard]] VkExtent2D getOffscreenExtent() const { return m_offscreenExtent; }

    std::unique_ptr<SwapChain>& getSwapChain() { return m_SwapChain; }

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

    VkFramebuffer m_offscreenFramebuffer = VK_NULL_HANDLE;
    VkRenderPass m_offscreenRenderPass = VK_NULL_HANDLE;
    VkSampler m_offscreenSampler = VK_NULL_HANDLE;
    VkDescriptorSet m_offscreenImGuiDescriptorSet = VK_NULL_HANDLE;
    VkDescriptorPool m_offscreenDescriptorPool = VK_NULL_HANDLE;
    VkImageLayout m_offscreenCurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkDescriptorSetLayout m_offscreenDescriptorSetLayout = VK_NULL_HANDLE;
    VkFormat m_offscreenFormat = VK_FORMAT_B8G8R8A8_UNORM;
    VkExtent2D m_offscreenExtent{};

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
    void createOffscreenRenderPass();
    void createOffscreenFramebuffer();
    void createOffscreenDescriptorSet();
    void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) const;
};

} // Namespace KaguEngine