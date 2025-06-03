module;

// libs
#include <vulkan/vulkan.h>

// std
import std;

import KaguEngine.Device;
import KaguEngine.SwapChain;

export module KaguEngine.Texture;

export namespace KaguEngine {

class Texture {
public:
    struct Material {
        VkDescriptorSet descriptorSet{};
        VkImageView textureView{};
        VkSampler textureSampler{};
    };

    Texture(Device &device, SwapChain &swapChain, const std::string &filepath,
            VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool);
    ~Texture();

    // Personalized texture
    static std::unique_ptr<Texture> createTextureFromFile(Device &device, SwapChain &swapChain,
                                                          const std::string &filepath,
                                                          VkDescriptorSetLayout descriptorSetLayout,
                                                          VkDescriptorPool descriptorPool);
    // Dummy texture 1x1 pixel
    static std::unique_ptr<Texture> makeDummyTexture(Device& device, SwapChain& swapChain,
                                                     VkDescriptorSetLayout descriptorSetLayout,
                                                     VkDescriptorPool descriptorPool);

    [[nodiscard]] const VkImage& getTextureImage()         const { return m_TextureImage; }
    [[nodiscard]] const VkDeviceMemory& getTextureMemory() const { return m_TextureImageMemory; }
    [[nodiscard]] const VkImageView& getTextureImageView() const { return m_TextureImageView; }
    [[nodiscard]] const VkSampler& getTextureSampler()     const { return m_TextureSampler; }
    [[nodiscard]] const Material& getMaterial()            const { return m_Material; }

private:
    void loadTexture(const std::string &filepath);
    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
                     VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory) const;
    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight,
                         uint32_t mipLevels) const;
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                               uint32_t mipLevels) const;
    void createTextureImageView();
    void createTextureSampler();
    void createMaterial(VkDescriptorSetLayout layout, VkDescriptorPool pool);

    uint32_t m_MipLevels;

    VkDeviceMemory m_TextureImageMemory{};
    VkImage m_TextureImage{};
    VkImageView m_TextureImageView{};
    VkSampler m_TextureSampler{};

    Device &deviceRef;
    SwapChain &swapChainRef;

    Material m_Material;
};

} // Namespace KaguEngine