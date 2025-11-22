module;

// libs
#include <vulkan/vulkan.h>

module KaguEngine.SwapChain;

// std
import std;

import KaguEngine.Device;

namespace KaguEngine {

SwapChain::SwapChain(Device &deviceRef, const VkExtent2D windowExtent) :
    deviceRef{deviceRef}, m_WindowExtent{windowExtent} {
    init();
}

SwapChain::SwapChain(Device &deviceRef, const VkExtent2D windowExtent, const std::shared_ptr<SwapChain> previous) :
    m_OldSwapChain{previous}, m_WindowExtent{windowExtent}, deviceRef{deviceRef} {
    init();
}

void SwapChain::init() {
    createSwapChain();
    createImageViews();
    createDepthResources();
    createColorResources();
    createSyncObjects();
}

SwapChain::~SwapChain() {
    // Destroy swapchain image views
    for (const auto imageView: m_SwapChainImageViews) {
        vkDestroyImageView(deviceRef.device(), imageView, nullptr);
    }
    m_SwapChainImageViews.clear();

    // Destroy multisample color image views, images, and memories
    for (size_t i = 0; i < m_MultisampleColorImages.size(); i++) {
        vkDestroyImageView(deviceRef.device(), m_MultisampleColorImageViews[i], nullptr);
        vkDestroyImage(deviceRef.device(), m_MultisampleColorImages[i], nullptr);
        vkFreeMemory(deviceRef.device(), m_MultisampleColorImageMemories[i], nullptr);
    }
    m_MultisampleColorImageViews.clear();
    m_MultisampleColorImages.clear();
    m_MultisampleColorImageMemories.clear();

    // Destroy depth image views, images, and memories
    for (size_t i = 0; i < m_DepthImages.size(); i++) {
        vkDestroyImageView(deviceRef.device(), m_DepthImageViews[i], nullptr);
        vkDestroyImage(deviceRef.device(), m_DepthImages[i], nullptr);
        vkFreeMemory(deviceRef.device(), m_DepthImageMemories[i], nullptr);
    }
    m_DepthImageViews.clear();
    m_DepthImages.clear();
    m_DepthImageMemories.clear();

    // Destroy synchronization objects
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(deviceRef.device(), m_ImageAvailableSemaphores[i], nullptr);
        vkDestroyFence(deviceRef.device(), m_InFlightFences[i], nullptr);
    }
    for (uint32_t i = 0; i < m_ImageCount; i++) {
        vkDestroySemaphore(deviceRef.device(), m_RenderFinishedSemaphores[i], nullptr);
    }
    m_RenderFinishedSemaphores.clear();
    m_ImageAvailableSemaphores.clear();
    m_InFlightFences.clear();
    m_ImagesInFlight.clear();

    // Destroy the swapchain
    if (m_SwapChain != nullptr) {
        vkDestroySwapchainKHR(deviceRef.device(), m_SwapChain, nullptr);
        m_SwapChain = nullptr;
    }
}

VkResult SwapChain::acquireNextImage(uint32_t *imageIndex) const {
    vkWaitForFences(deviceRef.device(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE,
                    std::numeric_limits<uint64_t>::max());

    auto start = std::chrono::high_resolution_clock::now();
    const VkResult result =
            vkAcquireNextImageKHR(deviceRef.device(), m_SwapChain, UINT64_MAX,
                                  m_ImageAvailableSemaphores[m_CurrentFrame], // must be a not signaled semaphore
                                  VK_NULL_HANDLE, imageIndex);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    if (duration.count() > 0)
        std::cout << "vkAcquireNextImageKHR in " << duration.count() << "ms" << std::endl;

    return result;
}

VkResult SwapChain::submitCommandBuffers(const VkCommandBuffer* buffers, const uint32_t* imageIndex) {
    if (m_ImagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(deviceRef.device(), 1, &m_ImagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
    }
    m_ImagesInFlight[*imageIndex] = m_InFlightFences[m_CurrentFrame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphore   = m_ImageAvailableSemaphores[m_CurrentFrame];
    VkSemaphore signalSemaphore = m_RenderFinishedSemaphores[*imageIndex];

    constexpr VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &waitSemaphore;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = buffers;

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &signalSemaphore;

    vkResetFences(deviceRef.device(), 1, &m_InFlightFences[m_CurrentFrame]);
    if (vkQueueSubmit(deviceRef.graphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &signalSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_SwapChain;
    presentInfo.pImageIndices = imageIndex;

    const auto result = vkQueuePresentKHR(deviceRef.presentQueue(), &presentInfo);

    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return result;
}

void SwapChain::createSwapChain() {
    const auto [capabilities, formats, presentModes] = deviceRef.getSwapChainSupport();

    const auto [format, colorSpace] = chooseSwapSurfaceFormat(formats);
    const VkPresentModeKHR presentMode = chooseSwapPresentMode(presentModes);
    const VkExtent2D extent = chooseSwapExtent(capabilities);

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }
    m_ImageCount = imageCount;

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = deviceRef.surface();

    createInfo.minImageCount = m_ImageCount;
    createInfo.imageFormat = format;
    createInfo.imageColorSpace = colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const QueueFamilyIndices indices = deviceRef.findPhysicalQueueFamilies();
    const uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = m_OldSwapChain == nullptr ? VK_NULL_HANDLE : m_OldSwapChain->m_SwapChain;

    if (m_OldSwapChain != nullptr && m_OldSwapChain->m_SwapChain == m_SwapChain) {
        std::cout << "SAME\n";
    }

    if (vkCreateSwapchainKHR(deviceRef.device(), &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    m_SwapChainImages.resize(m_ImageCount);
    vkGetSwapchainImagesKHR(deviceRef.device(), m_SwapChain, &m_ImageCount, m_SwapChainImages.data());

    m_SwapChainImageFormat = format;
    m_SwapChainExtent = extent;
}

VkImageView SwapChain::createImageView(const VkImage image, const VkFormat format, const VkImageAspectFlags aspectFlags,
    const uint32_t mipLevels) const {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(deviceRef.device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create texture image view!");
    }

    return imageView;
}

void SwapChain::createImageViews() {
    m_SwapChainImageViews.resize(m_SwapChainImages.size());

    for (uint32_t i = 0; i < m_SwapChainImages.size(); i++) {
        m_SwapChainImageViews[i] = createImageView(m_SwapChainImages[i], m_SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

void SwapChain::createDepthResources() {
    const VkFormat depthFormat = findDepthFormat();
    m_SwapChainDepthFormat = depthFormat;
    const auto [width, height] = getSwapChainExtent();

    m_DepthImages.resize(imageCount());
    m_DepthImageMemories.resize(imageCount());
    m_DepthImageViews.resize(imageCount());

    for (int i = 0; i < m_DepthImages.size(); i++) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = depthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.samples = deviceRef.getSampleCount();
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = 0;

        deviceRef.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DepthImages[i],
                                      m_DepthImageMemories[i]);

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_DepthImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = depthFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(deviceRef.device(), &viewInfo, nullptr, &m_DepthImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
    }
}

void SwapChain::createColorResources() {
    const VkFormat colorFormat = m_SwapChainImageFormat;

    m_MultisampleColorImages.resize(imageCount());
    m_MultisampleColorImageMemories.resize(imageCount());
    m_MultisampleColorImageViews.resize(imageCount());

    for (size_t i = 0; i < imageCount(); i++) {
        VkImageCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.imageType = VK_IMAGE_TYPE_2D;
        createInfo.extent.width = m_SwapChainExtent.width;
        createInfo.extent.height = m_SwapChainExtent.height;
        createInfo.extent.depth = 1;
        createInfo.mipLevels = 1;
        createInfo.arrayLayers = 1;
        createInfo.format = colorFormat;
        createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        createInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.samples = deviceRef.getSampleCount();
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.flags = 0;
        deviceRef.createImageWithInfo(createInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_MultisampleColorImages[i], m_MultisampleColorImageMemories[i]);
        m_MultisampleColorImageViews[i] = createImageView(
        m_MultisampleColorImages[i], colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

void SwapChain::createSyncObjects() {
    m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_RenderFinishedSemaphores.resize(m_ImageCount);
    m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    m_ImagesInFlight.assign(m_ImageCount, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(deviceRef.device(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(deviceRef.device(), &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create per-frame synchronization objects");
        }
    }

    for (uint32_t i = 0; i < m_ImageCount; i++) {
        if (vkCreateSemaphore(deviceRef.device(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create per-image renderFinished semaphore");
        }
    }
}

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    for (const auto &availableFormat: availableFormats) {
        // availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM for ImGui looking right
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
    for (const auto &availablePresentMode: availablePresentModes) {
        // Lower latency, but frames might be dropped
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    for (const auto &availablePresentMode : availablePresentModes) {
        // V-Sync off
        if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
          return availablePresentMode;
        }
    }

    // No frames dropped, higher latency
    std::cout << "Swapchain present mode falling back to FIFO (V-Sync)\n";
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    VkExtent2D actualExtent = m_WindowExtent;
    actualExtent.width =
        std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height =
        std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
}

VkFormat SwapChain::findDepthFormat() const {
    return deviceRef.findSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

} // Namespace KaguEngine