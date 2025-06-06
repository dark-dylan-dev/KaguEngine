module;

#include "include/imgui_includes.hpp"

module KaguEngine.ImGuiContext;

import KaguEngine.Device;
import KaguEngine.SwapChain;
import KaguEngine.Window;

namespace KaguEngine {

ImGuiContext::ImGuiContext(Window &window, SwapChain &swapChain, Device &device, std::unique_ptr<DescriptorPool> &pool) :
    poolRef{pool}, deviceRef{device}, swapChainRef{swapChain}, windowRef{window} {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    setupConfigFlags();
    setupStyle();
    setupContext();
}

ImGuiContext::~ImGuiContext() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiContext::setupConfigFlags() {
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
}

void ImGuiContext::setupStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    ImGui::StyleColorsDark();
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
    beginRender();
    onRender();
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    endRender();
}

void ImGuiContext::beginRender() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiContext::onRender() {
    ImGuiIO& io = ImGui::GetIO();
    // Demo window
    ImGui::ShowDemoWindow();
    // Other window
    {
        ImGui::Begin("Other window");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();
    }
}

void ImGuiContext::endRender() {
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

} // Namespace KaguEngine