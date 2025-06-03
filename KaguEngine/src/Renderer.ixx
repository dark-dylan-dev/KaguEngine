module;

// libs
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// std
#include <cassert>

// std
import std;

import KaguEngine.Device;
import KaguEngine.SwapChain;
import KaguEngine.Window;

export module KaguEngine.Renderer;

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
};

} // Namespace KaguEngine