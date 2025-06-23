module;

// libs
#include "include/imgui_includes.hpp"
#include <glm/glm.hpp>

module KaguEngine.ImGuiContext;

import KaguEngine.Camera;
import KaguEngine.Device;
import KaguEngine.Entity;
import KaguEngine.SwapChain;
import KaguEngine.Renderer;
import KaguEngine.Window;

static void check_result(VkResult err) {
    //std::cout << err << std::endl;
}

namespace KaguEngine {

ImGuiContext::ImGuiContext(
    Window &window, SwapChain &swapChain, Device &device,
    std::unique_ptr<DescriptorPool> &pool,
    Entity::Map& sceneEntities, std::vector<Entity>& views, Camera& camera
) :
    poolRef{pool}, viewsRef{views}, entitiesRef{sceneEntities}, cameraRef{camera}, deviceRef{device}, swapChainRef{swapChain}, windowRef{window} {
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
    ImVec4* colors = style.Colors;

    // Corners
    //style.WindowRounding = 8.0f;
    //style.ChildRounding = 8.0f;
    //style.FrameRounding = 6.0f;
    //style.PopupRounding = 6.0f;
    //style.ScrollbarRounding = 6.0f;
    //style.GrabRounding = 6.0f;
    style.TabRounding = 6.0f;

    // Colors
    colors[ImGuiCol_Text] =                    ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled] =            ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
    colors[ImGuiCol_WindowBg] =                ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_ChildBg] =                 ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_PopupBg] =                 ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] =                  ImVec4(0.43f, 0.50f, 0.56f, 0.50f);
    colors[ImGuiCol_BorderShadow] =            ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] =                 ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] =          ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
    colors[ImGuiCol_FrameBgActive] =           ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
    colors[ImGuiCol_TitleBg] =                 ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
    colors[ImGuiCol_TitleBgActive] =           ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] =        ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] =               ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] =             ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] =           ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] =    ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] =     ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark] =               ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] =              ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] =        ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] =                  ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_ButtonHovered] =           ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_ButtonActive] =            ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
    colors[ImGuiCol_Header] =                  ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
    colors[ImGuiCol_HeaderHovered] =           ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive] =            ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator] =               ImVec4(0.43f, 0.50f, 0.56f, 0.50f);
    colors[ImGuiCol_SeparatorHovered] =        ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
    colors[ImGuiCol_SeparatorActive] =         ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ResizeGrip] =              ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] =       ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] =        ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab] =                     ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_TabHovered] =              ImVec4(0.28f, 0.56f, 1.00f, 0.80f);
    colors[ImGuiCol_TabActive] =               ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_TabUnfocused] =            ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive] =      ImVec4(0.14f, 0.22f, 0.36f, 1.00f);
    colors[ImGuiCol_DockingPreview] =          ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg] =          ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines] =               ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] =        ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] =           ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] =    ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] =          ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] =          ImVec4(1.00f, 0.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] =            ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] =   ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] =       ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] =        ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/roboto/Roboto-Regular.ttf", 18.0f);
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
    init_info.MinImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
    init_info.MSAASamples = deviceRef.getSampleCount();
    init_info.ImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
    init_info.CheckVkResultFn = check_result;
    ImGui_ImplVulkan_Init(&init_info);
    ImGui_ImplVulkan_CreateFontsTexture();
}

void ImGuiContext::recreateSwapChain() const {
    ImGui_ImplVulkan_SetMinImageCount(SwapChain::MAX_FRAMES_IN_FLIGHT);
}

void ImGuiContext::render(const Renderer& renderer) {
    beginRender();
    onRender(renderer);
}

void ImGuiContext::beginRender() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiContext::onRender(const Renderer& renderer) {
    const ImGuiViewport* viewport = setupViewport();
    const ImGuiID dockspace_id = setDockspaceID();

    static bool dockspaceInitialized = false;
    if (!dockspaceInitialized) {
        initializeDockspace(dockspace_id, viewport);
        dockspaceInitialized = true;
    }

    moveSceneEntities();
    render3DScene(renderer);
    renderSceneSpecs();
    // Demo
    ImGui::ShowDemoWindow();
}

ImGuiViewport* ImGuiContext::setupViewport() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    return viewport;
}

ImGuiID ImGuiContext::setDockspaceID() {
    constexpr ImGuiWindowFlags hostFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                           ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                           ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                                           ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("DockSpaceHost", nullptr, hostFlags);
    ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_AutoHideTabBar);
    ImGui::End();
    ImGui::PopStyleVar();
    return dockspace_id;
}

void ImGuiContext::initializeDockspace(const ImGuiID& dockspace_id, const ImGuiViewport* viewport) {
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_AutoHideTabBar);
    ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);

    ImGuiID dock_id_demo, dock_id_specs, dock_id_viewport;
    ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, &dock_id_demo, &dock_id_viewport);
    ImGui::DockBuilderSplitNode(dock_id_viewport, ImGuiDir_Right, 0.25f, &dock_id_specs, &dock_id_viewport);

    // Set the flag using internal node access
    if (ImGuiDockNode* node = ImGui::DockBuilderGetNode(dock_id_demo))
        node->LocalFlags |= ImGuiDockNodeFlags_AutoHideTabBar;
    if (ImGuiDockNode* node = ImGui::DockBuilderGetNode(dock_id_specs))
        node->LocalFlags |= ImGuiDockNodeFlags_AutoHideTabBar;
    if (ImGuiDockNode* node = ImGui::DockBuilderGetNode(dock_id_viewport))
        node->LocalFlags |= ImGuiDockNodeFlags_AutoHideTabBar;

    ImGui::DockBuilderDockWindow("3D Scene", dock_id_viewport);
    ImGui::DockBuilderDockWindow("Scene specs", dock_id_specs);
    ImGui::DockBuilderDockWindow("Dear ImGui Demo", dock_id_demo);

    ImGui::DockBuilderFinish(dockspace_id);
}

void ImGuiContext::render3DScene(const Renderer& renderer) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("3D Scene", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove);
    const ImVec2 availableSpace = ImGui::GetContentRegionAvail();
    ImGui::Image(reinterpret_cast<ImTextureID>(renderer.getOffscreenImGuiDescriptorSet()), availableSpace);
    ImGui::End();
    ImGui::PopStyleVar();
}

inline void DrawVec3Control(const std::string& label, glm::vec3& values, const glm::vec3 resetValue = {0.0f, 0.0f, 0.0f}, const float columnWidth = 180.0f, const bool isLight = false) {
    ImGui::PushID(label.c_str());
    std::string  x = "X", y = "Y", z = "Z";
    if(isLight){ x = "R"; y = "G"; z = "B"; }
    const std::string hashX = "##" + x;
    const std::string hashY = "##" + y;
    const std::string hashZ = "##" + z;

    if (ImGui::BeginTable("Vec3Table", 2, ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, columnWidth);
        ImGui::TableSetupColumn("Controls", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s", label.c_str());

        ImGui::TableSetColumnIndex(1);

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

        const float lineHeight = ImGui::GetFont()->FontSize + ImGui::GetStyle().FramePadding.y * 2.0f;
        const ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

        // X
        if (ImGui::Button(x.c_str(), buttonSize))
            values.x = resetValue.x;
        ImGui::SameLine();
        ImGui::DragFloat(hashX.c_str(), &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();

        ImGui::SameLine();
        // Y
        if (ImGui::Button(y.c_str(), buttonSize))
            values.y = resetValue.y;
        ImGui::SameLine();
        ImGui::DragFloat(hashY.c_str(), &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();

        ImGui::SameLine();
        // Z
        if (ImGui::Button(z.c_str(), buttonSize))
            values.z = resetValue.z;
        ImGui::SameLine();
        ImGui::DragFloat(hashZ.c_str(), &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();

        ImGui::EndTable();
    }

    ImGui::PopID();
}

void ImGuiContext::moveSceneEntities() {
    static bool open = true;
    ImGui::Begin("Entities", &open);
    for (auto &[id, entity] : entitiesRef) {
        std::string headerLabel;
        if (entity.pointLight != nullptr) {
            headerLabel = "Point light " + std::to_string(id);
        }
        else {
            headerLabel = entity.name;
        }
        if (ImGui::CollapsingHeader(headerLabel.c_str())) {
            ImGui::PushID(static_cast<int>(id));

            if (entity.pointLight != nullptr) {
                DrawVec3Control("Color", entity.color, {0, 0, 0}, 180, true);
            }
            else {
                DrawVec3Control("Translation", entity.transform.translation);
                DrawVec3Control("Rotation",    entity.transform.rotation);
                DrawVec3Control("Scale",       entity.transform.scale);
            }

            ImGui::PopID();
        }
    }
    ImGui::End();
}

void ImGuiContext::renderSceneSpecs() {
    static float a = 1.f;
    static float b = 50.f;
    static float c = 100.f;
    ImGui::Begin("Scene specs", nullptr);
    ImGui::SliderFloat("Depth", &m_MaxDepth, 1.0f, 100.f);
    ImGui::SliderFloat("FOV-Y", &m_FovY, 30.f, 120.f);
    ImGui::SliderInt("Camera index", &m_camIdx, 0, 1);
    if (ImGui::CollapsingHeader("Models")) {
        ImGui::SliderFloat("Model", &a, 1.0f, 100.f);
    }
    if (ImGui::CollapsingHeader("Textures")) {
        ImGui::SliderFloat("Texture", &b, 1.0f, 100.f);
    }
    if (ImGui::CollapsingHeader("Point lights")) {
        ImGui::SliderFloat("Point light", &c, 1.0f, 100.f);
    }
    ImGui::End();
}

void ImGuiContext::onPresent(VkCommandBuffer commandBuffer) {
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    endRender();
}

void ImGuiContext::endRender() {
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

} // Namespace KaguEngine