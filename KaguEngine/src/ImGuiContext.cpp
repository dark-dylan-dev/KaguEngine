module;

#include "include/imgui_includes.hpp"

module KaguEngine.ImGuiContext;

import KaguEngine.Device;
import KaguEngine.SwapChain;
import KaguEngine.Window;

namespace KaguEngine {

ImGuiContext::ImGuiContext(Window &window, SwapChain &swapChain, Device &device, std::unique_ptr<DescriptorPool> &pool) :
    poolRef{pool}, deviceRef{device}, swapChainRef{swapChain}, windowRef{window}
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enables Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enables Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    setupContext();
}

ImGuiContext::~ImGuiContext() {
    ImGui_ImplVulkan_DestroyFontsTexture();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiContext::setupContext() const {
    const auto indices = deviceRef.findPhysicalQueueFamilies();
    ImGui_ImplGlfw_InitForVulkan(windowRef.getGLFWwindow(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = deviceRef.instance();
    init_info.PhysicalDevice = deviceRef.getPhysicalDevice();
    init_info.Device = deviceRef.device();
    init_info.QueueFamily = indices.graphicsFamily;
    init_info.Queue = deviceRef.graphicsQueue();
    init_info.RenderPass = swapChainRef.getRenderPass();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = poolRef->getDescriptorPool();
    init_info.Allocator = nullptr;
    init_info.Subpass = 0;
    init_info.MinImageCount = swapChainRef.MAX_FRAMES_IN_FLIGHT;
    init_info.MSAASamples = deviceRef.getSampleCount();
    init_info.ImageCount = swapChainRef.MAX_FRAMES_IN_FLIGHT;
    //init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info);

    ImGui_ImplVulkan_CreateFontsTexture();
}

void ImGuiContext::recreateSwapChain() const {
    ImGui_ImplVulkan_SetMinImageCount(swapChainRef.MAX_FRAMES_IN_FLIGHT);
}

void ImGuiContext::render(VkCommandBuffer commandBuffer) {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Draws stuff here
    ImGui::ShowDemoWindow();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

} // Namespace KaguEngine