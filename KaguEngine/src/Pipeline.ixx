module;

// libs
#include <vulkan/vulkan.h>

export module KaguEngine.Pipeline;

// std
import std;

import KaguEngine.Device;

export namespace KaguEngine {

struct PipelineConfigInfo {
    PipelineConfigInfo() = default;
    PipelineConfigInfo(const PipelineConfigInfo &) = delete;
    PipelineConfigInfo &operator=(const PipelineConfigInfo &) = delete;

    std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
    VkPipelineViewportStateCreateInfo viewportInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    std::vector<VkDynamicState> dynamicStateEnables;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo;
    VkPipelineLayout pipelineLayout = nullptr;
    VkFormat colorAttachmentFormat;
    VkFormat depthAttachmentFormat;
};

class Pipeline {
public:
    Pipeline(Device &device, const std::string &vertFilepath, const std::string &fragFilepath,
             const PipelineConfigInfo &configInfo);
    ~Pipeline();

    Pipeline(const Pipeline &) = delete;
    Pipeline &operator=(const Pipeline &) = delete;

    void bind(VkCommandBuffer commandBuffer) const;

    static void defaultPipelineConfigInfo(PipelineConfigInfo &configInfo, bool isTextured);
    static void enableAlphaBlending(PipelineConfigInfo &configInfo);
    static void enableMSAA(PipelineConfigInfo &configInfo, const VkSampleCountFlagBits &msaaLevel);

private:
    static std::vector<char> readFile(const std::string &filepath);

    void createGraphicsPipeline(const std::string &vertFilepath, const std::string &fragFilepath,
                                const PipelineConfigInfo &configInfo);

    void createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule) const;

    Device &m_Device;
    VkPipeline m_graphicsPipeline;
    VkShaderModule m_vertShaderModule;
    VkShaderModule m_fragShaderModule;
};

} // Namespace KaguEngine