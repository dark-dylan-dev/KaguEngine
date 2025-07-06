module;

// libs
#include "include/font_awesome.hpp"
#include "include/imgui_includes.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

module KaguEngine.ImGuiContext;

import KaguEngine.Camera;
import KaguEngine.Device;
import KaguEngine.Entity;
import KaguEngine.SwapChain;
import KaguEngine.Renderer;
import KaguEngine.Window;

namespace KaguEngine {

namespace UI_Helpers {
    static bool DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f) {
        bool value_changed = false;
        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];

        ImGui::PushID(label.c_str());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

        // X
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("X", buttonSize)) {
            values.x = resetValue;
            value_changed = true;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        if(ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f")) value_changed = true;
        ImGui::PopItemWidth();
        ImGui::SameLine();

        // Y
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Y", buttonSize)) {
            values.y = resetValue;
            value_changed = true;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        if(ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f")) value_changed = true;
        ImGui::PopItemWidth();
        ImGui::SameLine();

        // Z
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Z", buttonSize)) {
            values.z = resetValue;
            value_changed = true;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        if(ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f")) value_changed = true;
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();
        ImGui::Columns(1);
        ImGui::PopID();

        return value_changed;
    }
}

ImGuiContext::ImGuiContext(
    Window &window, SwapChain &swapChain, Device &device,
    std::unique_ptr<DescriptorPool> &pool,
    Entity::Map& sceneEntities, std::vector<Entity>& views, Camera& camera, glm::vec4& ambientLight
) :
    ambientLightColor{ambientLight}, poolRef{pool}, viewsRef{views}, entitiesRef{sceneEntities}, cameraRef{camera}, deviceRef{device}, swapChainRef{swapChain}, windowRef{window} {
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

    constexpr float baseFontSize = 18.0f;
    constexpr float iconFontSize = baseFontSize * 2.0f / 3.0f;
    io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/roboto/Roboto-Regular.ttf", baseFontSize);
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphMinAdvanceX = iconFontSize;
    static constexpr ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    io.Fonts->AddFontFromFileTTF(FONT_ICON_FILE_NAME_FA, iconFontSize, &icons_config, icons_ranges);

    ImGui::StyleColorsDark();
    setAppTheme();
    setStyleVars();
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
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = poolRef->getDescriptorPool();
    init_info.Allocator = nullptr;
    init_info.Subpass = 0;
    init_info.MinImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
    init_info.MSAASamples = deviceRef.getSampleCount();
    init_info.ImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
    init_info.UseDynamicRendering = true;
    ImGui_ImplVulkan_Init(&init_info);
    ImGui_ImplVulkan_CreateFontsTexture();
}

void ImGuiContext::setAppTheme() {
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg]           = ImVec4(0.149f, 0.149f,  0.149f, 1.0f);
    colors[ImGuiCol_Header]             = ImVec4{ 0.2f,  0.205f,  0.21f,  1.0f };
    colors[ImGuiCol_HeaderHovered]      = ImVec4{ 0.3f,  0.305f,  0.31f,  1.0f };
    colors[ImGuiCol_HeaderActive]       = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    colors[ImGuiCol_Button]             = ImVec4{ 0.2f,  0.205f,  0.21f,  1.0f };
    colors[ImGuiCol_ButtonHovered]      = ImVec4{ 0.3f,  0.305f,  0.31f,  1.0f };
    colors[ImGuiCol_ButtonActive]       = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    colors[ImGuiCol_FrameBg]            = ImVec4{ 0.2f,  0.205f,  0.21f,  1.0f };
    colors[ImGuiCol_FrameBgHovered]     = ImVec4{ 0.3f,  0.305f,  0.31f,  1.0f };
    colors[ImGuiCol_FrameBgActive]      = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    colors[ImGuiCol_Tab]                = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    colors[ImGuiCol_TabHovered]         = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
    colors[ImGuiCol_TabActive]          = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
    colors[ImGuiCol_TabUnfocused]       = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f,  0.205f,  0.21f,  1.0f };
    colors[ImGuiCol_TitleBg]            = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    colors[ImGuiCol_TitleBgActive]      = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    colors[ImGuiCol_TitleBgCollapsed]   = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
}

void ImGuiContext::setStyleVars() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding     = ImVec2(8, 8);
    style.FramePadding      = ImVec2(4, 4);
    style.ItemSpacing       = ImVec2(8, 4);
    style.ItemInnerSpacing  = ImVec2(4, 4);
    style.IndentSpacing     = 21.0f;
    style.ScrollbarSize     = 14.0f;
    style.GrabMinSize       = 12.0f;
    style.WindowBorderSize  = 1.0f;
    style.ChildBorderSize   = 1.0f;
    style.PopupBorderSize   = 1.0f;
    style.FrameBorderSize   = 0.0f;
    style.WindowRounding    = 0.0f;
    style.ChildRounding     = 0.0f;
    style.FrameRounding     = 0.0f;
    style.PopupRounding     = 2.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabRounding      = 2.0f;
    style.TabBorderSize     = 0.0f;
    style.TabRounding       = 0.0f;
    style.WindowTitleAlign  = ImVec2(0.5f, 0.5f);
    style.ButtonTextAlign   = ImVec2(0.5f, 0.5f);
}

void ImGuiContext::recreateSwapChain() {
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
    const ImGuiID dockspace_id = getDockspaceID();

    static bool dockspaceInitialized = false;
    if (!dockspaceInitialized) {
        initializeDockspace(dockspace_id, viewport);
        dockspaceInitialized = true;
    }

    // Render all UI panels
    render3DScene(renderer);
    renderSceneHierarchyPanel();
    renderPropertiesPanel();
    renderVisualsPanel();
    renderConsole();
}

ImGuiViewport* ImGuiContext::setupViewport() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    return viewport;
}

ImGuiID ImGuiContext::getDockspaceID() {
    constexpr ImGuiWindowFlags hostFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                           ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                           ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                                           ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("DockSpaceHost", nullptr, hostFlags);
    ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_AutoHideTabBar);
    drawMainMenuBar();
    ImGui::End();
    ImGui::PopStyleVar();
    return dockspace_id;
}

void ImGuiContext::drawMainMenuBar() {
    const ImGuiIO& io = ImGui::GetIO();
    ImGui::PushStyleVarY(ImGuiStyleVar_FramePadding, 5.f);
    ImGui::PushFont(io.Fonts->Fonts[0]);
    if (ImGui::BeginMainMenuBar()) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 8));
        if (ImGui::BeginMenu((std::string(ICON_FA_FILE) + " File").c_str())) {
            if (ImGui::MenuItem((std::string(ICON_FA_WINDOW_CLOSE) + " Exit").c_str(), "Ctrl+Q")) {
                m_IsRunning = false;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu((std::string(ICON_FA_TASKS) + " Window").c_str())) {
            if (ImGui::BeginMenu("Theme")) {
                if (ImGui::MenuItem((std::string("App Theme ")    + ICON_FA_GITHUB).c_str()))  {setAppTheme();setStyleVars();}
                if (ImGui::MenuItem((std::string("ImGui Dark ")   + ICON_FA_MOON_O).c_str()))  ImGui::StyleColorsDark();
                if (ImGui::MenuItem((std::string("ImGuiLight ")   + ICON_FA_SUN_O).c_str()))   ImGui::StyleColorsLight();
                if (ImGui::MenuItem((std::string("ImGuiClassic ") + ICON_FA_SMILE_O).c_str())) ImGui::StyleColorsClassic();
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::PopStyleVar();
        ImGui::EndMainMenuBar();
    }
    ImGui::PopFont();
    ImGui::PopStyleVar();
}

void ImGuiContext::initializeDockspace(const ImGuiID& dockspace_id, const ImGuiViewport* viewport) {
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);

    // Main dockspace: left panel and a "main" area
    ImGuiID dock_id_left, dock_id_main;
    ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, &dock_id_left, &dock_id_main);

    // "main" area: right panel and a "center" area
    ImGuiID dock_id_right, dock_id_center;
    ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Right, 0.25f, &dock_id_right, &dock_id_center);

    // "center" area: viewport on top, console on bottom
    ImGuiID dock_id_viewport, dock_id_console;
    ImGui::DockBuilderSplitNode(dock_id_center, ImGuiDir_Down, 0.25f, &dock_id_console, &dock_id_viewport);

    // Left panel: hierarchy on top, properties on bottom
    ImGuiID dock_id_hierarchy, dock_id_properties;
    ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.5f, &dock_id_properties, &dock_id_hierarchy);

    // Dock windows
    ImGui::DockBuilderDockWindow("Scene Hierarchy", dock_id_hierarchy);
    ImGui::DockBuilderDockWindow("Properties", dock_id_properties);
    ImGui::DockBuilderDockWindow("3D Scene", dock_id_viewport);
    ImGui::DockBuilderDockWindow("Visuals", dock_id_right);
    ImGui::DockBuilderDockWindow("Console", dock_id_console);

    ImGui::DockBuilderFinish(dockspace_id);
}

void ImGuiContext::render3DScene(const Renderer& renderer) const {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("3D Scene", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    const float fovx = getFovX();
    const float fovy = getFovY();
    const float nativeAspect = tan(fovx / 2.0f) / tan(fovy / 2.0f);
    const ImVec2 viewportSpace = ImGui::GetContentRegionAvail();
    const float viewportAspect = viewportSpace.x / viewportSpace.y;

    float idealWidth = viewportSpace.y * nativeAspect;
    float idealHeight = viewportSpace.x / nativeAspect;

    ImVec2 renderSize;
    if (viewportAspect > nativeAspect) {
        renderSize = { viewportSpace.x, idealHeight };
    } else {
        renderSize = { idealWidth, viewportSpace.y };
    }

    ImVec2 imagePos = {
        (viewportSpace.x - renderSize.x) * 0.5f,
        (viewportSpace.y - renderSize.y) * 0.5f
    };
    ImGui::SetCursorPos(imagePos);
    ImGui::Image(reinterpret_cast<ImTextureID>(renderer.getOffscreenImGuiDescriptorSet()), renderSize);

    ImGui::End();
    ImGui::PopStyleVar();
}

void ImGuiContext::renderSceneHierarchyPanel() {
    ImGui::Begin("Scene Hierarchy");

    if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen)) {
        for (auto& [id, entity] : entitiesRef) {
            std::string name = entity.pointLight != nullptr ? std::string(ICON_FA_LIGHTBULB_O) + " " : std::string(ICON_FA_CUBE) + " ";
            name += entity.name;

            bool is_selected = (m_SelectedEntityID == id);
            if (ImGui::Selectable(name.c_str(), is_selected)) {
                m_SelectedEntityID = id;
            }
        }
        ImGui::TreePop();
    }

    // Deselect if clicking in empty space
    if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
        m_SelectedEntityID = std::numeric_limits<Entity::id_t>::max();
    }

    ImGui::End();
}

void ImGuiContext::renderPropertiesPanel() {
    ImGui::Begin("Properties");

    if (m_SelectedEntityID != std::numeric_limits<Entity::id_t>::max()) {
        auto& entity = entitiesRef.at(m_SelectedEntityID);

        // Name
        char buffer[256];
        strncpy(buffer, entity.name.c_str(), sizeof(buffer));
        if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
            entity.name = std::string(buffer);
        }

        // Transform Component
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            UI_Helpers::DrawVec3Control("Translation", entity.transform.translation, 0.0f);
            UI_Helpers::DrawVec3Control("Rotation", entity.transform.rotation, 0.0f);
            UI_Helpers::DrawVec3Control("Scale", entity.transform.scale, 1.0f);
        }

        // Material/Color Component
        if (entity.texture == nullptr && entity.pointLight == nullptr) {
            if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
                 ImGui::ColorEdit3("Color", glm::value_ptr(entity.color));
            }
        }

        // Point Light Component
        if (entity.pointLight) {
            if (ImGui::CollapsingHeader("Point Light", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::ColorEdit3("Color", glm::value_ptr(entity.color));
                ImGui::DragFloat("Intensity", &entity.pointLight->lightIntensity, 0.1f, 0.0f, 100.0f);
            }
        }
    } else {
        ImGui::Text("Select an entity to view its properties.");
    }

    ImGui::End();
}

void ImGuiContext::renderVisualsPanel() {
    ImGui::Begin("Visuals");

    ImGui::Text("Camera");
    ImGui::SliderInt("##Camera", &m_CamIdx, 0, 1, "View %d");
    ImGui::DragFloat("Depth", &m_MaxDepth[m_CamIdx], 1.0f, 1.0f, 100.f);
    ImGui::DragFloat("FOV-X", &m_FovX[m_CamIdx], 1.0f, 30.f, 120.f);
    ImGui::DragFloat("FOV-Y", &m_FovY[m_CamIdx], 1.0f, 30.f, 120.f);

    ImGui::Separator();

    ImGui::Text("Lighting");
    ImGui::ColorEdit4("Ambient Light", glm::value_ptr(ambientLightColor));

    ImGui::End();
}

void ImGuiContext::renderConsole() {
    ImGui::Begin("Console", &m_ConsoleOpened);

    if (ImGui::SmallButton("Clear")) { m_Items.clear(); }
    ImGui::Separator();

    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar)) {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));
        for (auto& item : m_Items) {
            ImVec4 color = { 1.0f, 1.0f, 1.0f, 1.0f }; // Default
            if (strstr(item.c_str(), "[Error]")) { color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); }
            else if (strstr(item.c_str(), "[Hint]")) { color = ImVec4(0.6f, 0.8f, 1.0f, 1.0f); }
            else if (strstr(item.c_str(), "[Info]")) { color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f); }
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextUnformatted(item.c_str());
            ImGui::PopStyleColor();
        }

        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::PopStyleVar();
    }
    ImGui::EndChild();
    ImGui::Separator();

    bool reclaim_focus = false;
    if (ImGui::InputText("Input", m_InputBuffer, IM_ARRAYSIZE(m_InputBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
        if (m_InputBuffer[0] != '\0') {
            m_Items.emplace_back(std::string("> ") + m_InputBuffer);
            // Command processing if needed later happens here
            memset(m_InputBuffer, 0, sizeof(m_InputBuffer));
        }
        reclaim_focus = true;
    }

    ImGui::SetItemDefaultFocus();
    if (reclaim_focus)
        ImGui::SetKeyboardFocusHere(-1);

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