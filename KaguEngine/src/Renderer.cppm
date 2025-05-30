module;

// libs
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// std
#include <array>
#include <cassert>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <vector>

export module Renderer;

import Device;
import SwapChain;
import Window;

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

// .cpp part

export namespace KaguEngine {

Renderer::Renderer(Window &window, Device &device) : windowRef{window}, deviceRef{device} {
    recreateSwapChain();
    createCommandBuffers();
}

Renderer::~Renderer() { freeCommandBuffers(); }

void Renderer::recreateSwapChain() {
    auto extent = windowRef.getExtent();
    while (extent.width == 0 || extent.height == 0) {
        extent = windowRef.getExtent();
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(deviceRef.device());

    if (m_SwapChain == nullptr) {
        m_SwapChain = std::make_unique<SwapChain>(deviceRef, extent);
    } else {
        std::shared_ptr<SwapChain> oldSwapChain = std::move(m_SwapChain);
        m_SwapChain = std::make_unique<SwapChain>(deviceRef, extent, oldSwapChain);
        if (!oldSwapChain->compareSwapFormats(*m_SwapChain)) {
            throw std::runtime_error("Swap chain image(or depth) format has changed!");
        }
    }
}

void Renderer::createCommandBuffers() {
    m_commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = deviceRef.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    if (vkAllocateCommandBuffers(deviceRef.device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void Renderer::freeCommandBuffers() {
    vkFreeCommandBuffers(deviceRef.device(), deviceRef.getCommandPool(),
                         static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
    m_commandBuffers.clear();
}

VkCommandBuffer Renderer::beginFrame() {
    assert(!m_isFrameStarted && "Can't call beginFrame while already in progress");

    const auto result = m_SwapChain->acquireNextImage(&m_currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return nullptr;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    m_isFrameStarted = true;

    const auto commandBuffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
    return commandBuffer;
}

void Renderer::endFrame() {
    assert(m_isFrameStarted && "Can't call endFrame while frame is not in progress");
    const auto commandBuffer = getCurrentCommandBuffer();
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

    if (const auto result = m_SwapChain->submitCommandBuffers(&commandBuffer, &m_currentImageIndex);
        result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || windowRef.windowResized()) {
        windowRef.resetWindowResizedFlag();
        recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    m_isFrameStarted = false;
    m_currentFrameIndex = (m_currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::beginSwapChainRenderPass(const VkCommandBuffer commandBuffer) const {
    assert(m_isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() &&
           "Can't begin render pass on command buffer from a different frame");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_SwapChain->getRenderPass();
    renderPassInfo.framebuffer = m_SwapChain->getFrameBuffer(m_currentImageIndex);

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_SwapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.1f, 0.1f, 0.15f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_SwapChain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(m_SwapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    const VkRect2D scissor{{0, 0}, m_SwapChain->getSwapChainExtent()};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void Renderer::endSwapChainRenderPass(const VkCommandBuffer commandBuffer) const {
    assert(m_isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() &&
           "Can't end render pass on command buffer from a different frame");
    vkCmdEndRenderPass(commandBuffer);
}

} // Namespace KaguEngine
