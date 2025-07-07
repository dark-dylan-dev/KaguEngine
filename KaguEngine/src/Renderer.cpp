module;

// libs
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// std
#include <cassert>

module KaguEngine.Renderer;

// std
import std;

import KaguEngine.Device;
import KaguEngine.SwapChain;
import KaguEngine.Window;

namespace KaguEngine {

namespace { // Anonymous namespace for internal helpers
void cmdTransitionImageLayout(
    VkCommandBuffer commandBuffer,
    VkImage image,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    VkImageSubresourceRange subresourceRange,
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    VkAccessFlags srcAccessMask,
    VkAccessFlags dstAccessMask) {

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange = subresourceRange;
    barrier.srcAccessMask = srcAccessMask;
    barrier.dstAccessMask = dstAccessMask;

    vkCmdPipelineBarrier(
        commandBuffer,
        srcStageMask,
        dstStageMask,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);
}
}

Renderer::Renderer(Window &window, Device &device) : windowRef{window}, deviceRef{device} {
    m_currentImageIndex = 0;
    recreateSwapChain();
    createOffscreenResources();
    createCommandBuffers();
}

Renderer::~Renderer() {
    freeCommandBuffers();
    cleanupOffscreenResources();
}

void Renderer::recreateSwapChain() {
    auto extent = windowRef.getExtent();
    while (extent.width == 0 || extent.height == 0) {
        extent = windowRef.getExtent();
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(deviceRef.device());
    cleanupOffscreenResources();
    if (m_SwapChain == nullptr) {
        m_SwapChain = std::make_unique<SwapChain>(deviceRef, extent);
    } else {
        std::shared_ptr<SwapChain> oldSwapChain = std::move(m_SwapChain);
        m_SwapChain = std::make_unique<SwapChain>(deviceRef, extent, oldSwapChain);
        if (!oldSwapChain->compareSwapFormats(*m_SwapChain)) {
            throw std::runtime_error("Swap chain image(or depth) format has changed!");
        }
    }
    createOffscreenResources();
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

void Renderer::beginSwapChainRendering(const VkCommandBuffer commandBuffer) const {
    assert(m_isFrameStarted && "Can't call beginSwapChainRendering if frame is not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin rendering on command buffer from a different frame");

    VkImageSubresourceRange subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    cmdTransitionImageLayout(
        commandBuffer,
        m_SwapChain->getImage(m_currentImageIndex),
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        subresourceRange,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView = m_SwapChain->getMultisampleColorImageView(m_currentImageIndex);
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    colorAttachment.resolveImageView = m_SwapChain->getImageView(m_currentImageIndex);
    colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = { clearColor.r, clearColor.g, clearColor.b, clearColor.a };

    VkRenderingAttachmentInfo depthAttachment{};
    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachment.imageView = m_SwapChain->getDepthImageView(m_currentImageIndex);
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.clearValue.depthStencil = {1.0f, 0};

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea = {{0, 0}, m_SwapChain->getSwapChainExtent()};
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;
    renderingInfo.pDepthAttachment = &depthAttachment;
    renderingInfo.pStencilAttachment = nullptr;

    vkCmdBeginRendering(commandBuffer, &renderingInfo);

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

void Renderer::endSwapChainRendering(const VkCommandBuffer commandBuffer) const {
    assert(m_isFrameStarted && "Can't call endSwapChainRendering if frame is not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() && "Can't end rendering on command buffer from a different frame");

    vkCmdEndRendering(commandBuffer);

    VkImageSubresourceRange subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    cmdTransitionImageLayout(
        commandBuffer,
        m_SwapChain->getImage(m_currentImageIndex),
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        subresourceRange,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        0);
}

void Renderer::createOffscreenResources() {
    cleanupOffscreenResources();

    m_offscreenExtent = m_SwapChain->getSwapChainExtent();
    m_offscreenFormat = VK_FORMAT_B8G8R8A8_UNORM;

    // Create color attachment
    VkImageCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.imageType = VK_IMAGE_TYPE_2D;
    createInfo.extent.width = m_offscreenExtent.width;
    createInfo.extent.height = m_offscreenExtent.height;
    createInfo.extent.depth = 1;
    createInfo.mipLevels = 1;
    createInfo.arrayLayers = 1;
    createInfo.format = m_offscreenFormat;
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    createInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.samples = deviceRef.getSampleCount();
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.flags = 0;

    // Create color resolve attachment
    VkImageCreateInfo colorResolveCreateInfo{};
    colorResolveCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    colorResolveCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    colorResolveCreateInfo.extent.width = m_offscreenExtent.width;
    colorResolveCreateInfo.extent.height = m_offscreenExtent.height;
    colorResolveCreateInfo.extent.depth = 1;
    colorResolveCreateInfo.mipLevels = 1;
    colorResolveCreateInfo.arrayLayers = 1;
    colorResolveCreateInfo.format = m_offscreenFormat;
    colorResolveCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    colorResolveCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorResolveCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    colorResolveCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    colorResolveCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    colorResolveCreateInfo.flags = 0;

    // Multi sampled color
    deviceRef.createImageWithInfo(createInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_offscreenImage, m_offscreenImageMemory);
    m_offscreenImageView = m_SwapChain->createImageView(m_offscreenImage, m_offscreenFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);

    // Resolve (single sampled)
    deviceRef.createImageWithInfo(colorResolveCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_offscreenResolveImage, m_offscreenResolveMemory);
    m_offscreenResolveImageView = m_SwapChain->createImageView(m_offscreenResolveImage, m_offscreenFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);

    // Create sampler
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    if (vkCreateSampler(deviceRef.device(), &samplerInfo, nullptr, &m_offscreenSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create offscreen sampler!");
    }

    // Create depth image
    m_offscreenDepthFormat = m_SwapChain->findDepthFormat();
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = m_offscreenExtent.width;
    imageInfo.extent.height = m_offscreenExtent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = m_offscreenDepthFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = deviceRef.getSampleCount();
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;

    deviceRef.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_offscreenDepthImage, m_offscreenDepthMemory);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_offscreenDepthImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = m_offscreenDepthFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(deviceRef.device(), &viewInfo, nullptr, &m_offscreenDepthView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    // Initial layout transition
    auto cmd = deviceRef.beginSingleTimeCommands();
    VkImageSubresourceRange subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    cmdTransitionImageLayout(
        cmd,
        m_offscreenResolveImage,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        subresourceRange,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, VK_ACCESS_SHADER_READ_BIT);
    deviceRef.endSingleTimeCommands(cmd);

    m_offscreenCurrentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    createOffscreenDescriptorSet();
}

void Renderer::cleanupOffscreenResources() {
    const auto device = deviceRef.device();

    if (m_offscreenImGuiDescriptorSet) {
        // ImGui will clean up its descriptor set, do not free here
        m_offscreenImGuiDescriptorSet = VK_NULL_HANDLE;
    }

    if (m_offscreenDescriptorPool) {
        vkDestroyDescriptorPool(device, m_offscreenDescriptorPool, nullptr);
        m_offscreenDescriptorPool = VK_NULL_HANDLE;
    }
    if (m_offscreenDescriptorSetLayout) {
        vkDestroyDescriptorSetLayout(device, m_offscreenDescriptorSetLayout, nullptr);
        m_offscreenDescriptorSetLayout = VK_NULL_HANDLE;
    }
    if (m_offscreenImageView) {
        vkDestroyImageView(device, m_offscreenImageView, nullptr);
        m_offscreenImageView = VK_NULL_HANDLE;
    }
    if (m_offscreenResolveImageView) {
        vkDestroyImageView(device, m_offscreenResolveImageView, nullptr);
        m_offscreenResolveImageView = VK_NULL_HANDLE;
    }
    if (m_offscreenResolveImage) {
        vkDestroyImage(device, m_offscreenResolveImage, nullptr);
        m_offscreenResolveImage = VK_NULL_HANDLE;
    }
    if (m_offscreenImage) {
        vkDestroyImage(device, m_offscreenImage, nullptr);
        m_offscreenImage = VK_NULL_HANDLE;
    }
    if (m_offscreenResolveMemory) {
        vkFreeMemory(device, m_offscreenResolveMemory, nullptr);
        m_offscreenResolveMemory = VK_NULL_HANDLE;
    }
    if (m_offscreenImageMemory) {
        vkFreeMemory(device, m_offscreenImageMemory, nullptr);
        m_offscreenImageMemory = VK_NULL_HANDLE;
    }
    if (m_offscreenSampler) {
        vkDestroySampler(device, m_offscreenSampler, nullptr);
        m_offscreenSampler = VK_NULL_HANDLE;
    }
    if (m_offscreenDepthView) {
        vkDestroyImageView(device, m_offscreenDepthView, nullptr);
        m_offscreenDepthView = VK_NULL_HANDLE;
    }
    if (m_offscreenDepthImage) {
        vkDestroyImage(device, m_offscreenDepthImage, nullptr);
        m_offscreenDepthImage = VK_NULL_HANDLE;
    }
    if (m_offscreenDepthMemory) {
        vkFreeMemory(device, m_offscreenDepthMemory, nullptr);
        m_offscreenDepthMemory = VK_NULL_HANDLE;
    }
}

void Renderer::createOffscreenDescriptorSet() {
    const auto device = deviceRef.device();

    // Descriptor pool for one sampled image
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;
    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_offscreenDescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool for offscreen image!");
    }

    // Descriptor set layout for one combined sampler
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &samplerLayoutBinding;
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_offscreenDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout for offscreen image!");
    }

    // Allocate descriptor set
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_offscreenDescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_offscreenDescriptorSetLayout;
    if (vkAllocateDescriptorSets(device, &allocInfo, &m_offscreenImGuiDescriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor set for offscreen image!");
    }

    // Update descriptor set for the offscreen image
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_offscreenResolveImageView;
    imageInfo.sampler = m_offscreenSampler;

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_offscreenImGuiDescriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
}

void Renderer::beginOffscreenRendering(VkCommandBuffer commandBuffer) {
    if (m_offscreenCurrentLayout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        VkImageSubresourceRange subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        cmdTransitionImageLayout(
            commandBuffer,
            m_offscreenResolveImage,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            subresourceRange,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
        m_offscreenCurrentLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView = m_offscreenImageView;
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    colorAttachment.resolveImageView = m_offscreenResolveImageView;
    colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = { clearColor.r, clearColor.g, clearColor.b, clearColor.a };

    VkRenderingAttachmentInfo depthAttachment{};
    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachment.imageView = m_offscreenDepthView;
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.clearValue.depthStencil = {1.0f, 0};

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea = {{0, 0}, m_offscreenExtent};
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;
    renderingInfo.pDepthAttachment = &depthAttachment;
    renderingInfo.pStencilAttachment = nullptr;

    vkCmdBeginRendering(commandBuffer, &renderingInfo);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_offscreenExtent.width);
    viewport.height = static_cast<float>(m_offscreenExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    const VkRect2D scissor{{0, 0}, m_offscreenExtent};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void Renderer::endOffscreenRendering(VkCommandBuffer commandBuffer) const {
    vkCmdEndRendering(commandBuffer);
}

void Renderer::transitionOffscreenImageForImGui(VkCommandBuffer commandBuffer) {
    if (m_offscreenCurrentLayout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        VkImageSubresourceRange subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        cmdTransitionImageLayout(
            commandBuffer,
            m_offscreenResolveImage,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            subresourceRange,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT);
        m_offscreenCurrentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
}

} // Namespace KaguEngine