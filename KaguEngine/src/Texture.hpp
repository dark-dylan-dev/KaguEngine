#pragma once

#include "Device.hpp"
#include "SwapChain.hpp"

// std
#include <memory>

namespace KaguEngine {

class Texture {
public:
    struct Material {
        VkDescriptorSet descriptorSet{};
        VkImageView textureView{};
        VkSampler textureSampler{};
    };

    Texture(Device& device, SwapChain& swapChain, const std::string& filepath,
            VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool);
    ~Texture();

    static std::unique_ptr<Texture> createTextureFromFile(Device& device, SwapChain& swapChain,
                                                          const std::string& filepath,
                                                          VkDescriptorSetLayout descriptorSetLayout,
                                                          VkDescriptorPool descriptorPool);

    const VkImage& getTextureImage() const { return textureImage; }
    const VkDeviceMemory& getTextureMemory() const { return textureImageMemory; }
    const VkImageView& getTextureImageView() const { return textureImageView; }
    const VkSampler& getTextureSampler() const { return textureSampler; }

    const Material& getMaterial() const { return material; }

private:
    void loadTexture(const std::string& filepath);
    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
                     VkSampleCountFlagBits numSamples, VkFormat format,
                     VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties,
                     VkImage& image, VkDeviceMemory& imageMemory) const;
    void generateMipmaps(VkImage image, VkFormat imageFormat,
                         int32_t texWidth, int32_t texHeight, uint32_t mipLevels) const;
    void transitionImageLayout(VkImage image, VkFormat format,
                               VkImageLayout oldLayout, VkImageLayout newLayout,
                               uint32_t mipLevels) const;
    void createTextureImageView();
    void createTextureSampler();
    void createMaterial(VkDescriptorSetLayout layout, VkDescriptorPool pool);

    uint32_t mipLevels;

    VkDeviceMemory textureImageMemory{};
    VkImage textureImage{};
    VkImageView textureImageView{};
    VkSampler textureSampler{};

    Device& deviceRef;
    SwapChain& swapChainRef;

    Material material;
};

} // namespace KaguEngine