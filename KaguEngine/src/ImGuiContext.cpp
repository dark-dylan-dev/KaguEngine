module;

// libs
#include "include/font_awesome.hpp"
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

    // Fonts sizes
    constexpr float baseFontSize = 18.0f;
    constexpr float iconFontSize = baseFontSize * 2.0f / 3.0f;
    // Base regular font
    io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/roboto/Roboto-Regular.ttf", baseFontSize);
    // Icons font
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphMinAdvanceX = iconFontSize;
    static constexpr ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    io.Fonts->AddFontFromFileTTF(FONT_ICON_FILE_NAME_FA, iconFontSize, &icons_config, icons_ranges);

    // Theme + Style
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

void ImGuiContext::setStyleVars() {
    ImGuiStyle& style = ImGui::GetStyle();

    // === Main ===
    style.WindowPadding        = ImVec2(8, 8);
    style.FramePadding         = ImVec2(4, 4);
    style.ItemSpacing          = ImVec2(8, 4);
    style.ItemInnerSpacing     = ImVec2(4, 4);
    style.TouchExtraPadding    = ImVec2(4, 4);
    style.IndentSpacing        = 21.0f;
    style.ScrollbarSize        = 14.0f;
    style.GrabMinSize          = 12.0f;

    // === Borders ===
    style.WindowBorderSize     = 1.0f;
    style.ChildBorderSize      = 0.0f;
    style.PopupBorderSize      = 1.0f;
    style.FrameBorderSize      = 0.0f;

    // === Rounding ===
    style.WindowRounding       = 0.0f;
    style.ChildRounding        = 0.0f;
    style.FrameRounding        = 8.0f;
    style.PopupRounding        = 8.0f;
    style.ScrollbarRounding    = 8.0f;
    style.GrabRounding         = 8.0f;

    // === Tabs ===
    style.TabBorderSize                    = 0.0f;
    style.TabBarBorderSize                 = 0.0f;
    style.TabBarOverlineSize               = 0.0f;
    style.TabCloseButtonMinWidthSelected   = -1.0f; // Always
    style.TabCloseButtonMinWidthUnselected = 0.0f;
    style.TabRounding                      = 0.0f;

    // === Tables ===
    style.CellPadding                       = ImVec2(4, 2);
    style.TableAngledHeadersAngle           = 35.0f; // degrees
    style.TableAngledHeadersTextAlign       = ImVec2(0.50f, 0.00f); // center

    // === Trees ===
    style.TreeLinesFlags         = ImGuiTreeNodeFlags_None; // DrawLinesNone
    style.TreeLinesSize          = 0.0f;
    style.TreeLinesRounding      = 0.0f;

    // === Windows ===
    style.WindowTitleAlign           = ImVec2(0.00f, 0.50f);
    style.WindowMenuButtonPosition   = ImGuiDir_None;
    style.WindowBorderHoverPadding   = 5.0f;

    // === Widgets ===
    style.ColorButtonPosition        = ImGuiDir_Right;
    style.ButtonTextAlign            = ImVec2(0.50f, 0.50f);
    style.SelectableTextAlign        = ImVec2(0.00f, 0.00f);
    style.SeparatorTextBorderSize    = 3.0f;
    style.SeparatorTextAlign         = ImVec2(0.00f, 0.50f);
    style.SeparatorTextPadding       = ImVec2(20.0f, 3.0f);
    style.LogSliderDeadzone          = 4.0f;
    style.ImageBorderSize            = 0.0f;

    // === Docking ===
    style.DockingSeparatorSize       = 2.0f;

    // === Misc ===
    style.DisplayWindowPadding   = ImVec2(19, 19);
    style.DisplaySafeAreaPadding = ImVec2(3, 3);
}

void ImGuiContext::setAppTheme() {
    ImVec4* colors = ImGui::GetStyle().Colors;

    // ---- Text ----
    colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);    // Main text (white)
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.56f, 0.59f, 0.65f, 1.00f);    // Disabled text (gray)

    // ---- Backgrounds ----
    colors[ImGuiCol_WindowBg]               = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);    // Window background
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);    // Child window background
    colors[ImGuiCol_PopupBg]                = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);    // Popup/tooltip background

    // ---- Borders ----
    colors[ImGuiCol_Border]                 = ImVec4(0.20f, 0.20f, 0.23f, 1.00f);    // Borders
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);    // Border shadow (unused)

    // ---- Frame Backgrounds (Inputs, etc) ----
    colors[ImGuiCol_FrameBg]                = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);    // Input fields, etc.
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);    // Input hover
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);    // Input active

    // ---- Title Bar ----
    colors[ImGuiCol_TitleBg]                = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);    // Title bar background (inactive/active)
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);    // Title bar background (active)
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);    // Title bar background (collapsed)

    // ---- Menu Bar ----
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);    // Menu bar background

    // ---- Scrollbar ----
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.11f, 0.12f, 0.13f, 1.00f);    // Scrollbar background
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.18f, 0.20f, 0.25f, 1.00f);    // Scrollbar grab
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.30f, 0.31f, 0.37f, 1.00f);    // Scrollbar grab (hover)
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.52f, 0.53f, 0.57f, 1.00f);    // Scrollbar grab (active)

    // ---- Controls: Check, Slider, Button ----
    colors[ImGuiCol_CheckMark]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);    // Checkbox check mark (white)
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.46f, 0.49f, 0.55f, 1.00f);    // Slider grab
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.27f, 0.30f, 0.38f, 1.00f);    // Slider grab (active)

    colors[ImGuiCol_Button]                 = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);    // Button
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.25f, 0.31f, 0.44f, 1.00f);    // Button (hover)
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.27f, 0.39f, 0.64f, 1.00f);    // Button (active)

    // ---- Headers & Collapsing ----
    colors[ImGuiCol_Header]                 = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);    // Header (tree, table, etc.)
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.25f, 0.31f, 0.44f, 1.00f);    // Header (hover)
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.27f, 0.39f, 0.64f, 1.00f);    // Header (active)

    // ---- Separators ----
    colors[ImGuiCol_Separator]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);    // Separator line
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);    // Separator (hover)
    colors[ImGuiCol_SeparatorActive]        = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);    // Separator (active)

    // ---- Resize Grips ----
    colors[ImGuiCol_ResizeGrip]             = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);    // Resize grip
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);    // Resize grip (hover)
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);    // Resize grip (active)

    // ---- Input Text Cursor ----
    colors[ImGuiCol_InputTextCursor]        = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);    // Input text cursor

    // ---- Tabs ----
    colors[ImGuiCol_Tab]                    = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);    // Tab background
    colors[ImGuiCol_TabHovered]             = ImVec4(0.26f, 0.33f, 0.48f, 1.00f);    // Tab (hover)
    colors[ImGuiCol_TabSelected]            = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);    // Tab (selected)
    colors[ImGuiCol_TabSelectedOverline]    = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);    // Tab (selected overline)
    colors[ImGuiCol_TabDimmed]              = ImVec4(0.12f, 0.13f, 0.14f, 1.00f);    // Tab (dimmed)
    colors[ImGuiCol_TabDimmedSelected]      = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);    // Tab (dimmed selected)
    colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f); // Tab (dimmed selected overline)

    // ---- Docking ----
    colors[ImGuiCol_DockingPreview]         = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);    // Docking preview highlight
    colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.11f, 0.12f, 0.13f, 1.00f);    // Dockspace background

    // ---- Plots ----
    colors[ImGuiCol_PlotLines]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);    // Line plot
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);    // Line plot (hover)
    colors[ImGuiCol_PlotHistogram]          = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);    // Histogram bar
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);    // Histogram bar (hover)

    // ---- Tables ----
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.17f, 0.20f, 0.27f, 1.00f);    // Table header
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.18f, 0.21f, 0.30f, 1.00f);    // Table strong border
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.16f, 0.18f, 0.24f, 1.00f);    // Table light border
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.15f, 0.16f, 0.20f, 1.00f);    // Table row
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(0.15f, 0.16f, 0.20f, 1.00f);    // Table row (alternate)

    // ---- Misc/Links/Selection ----
    colors[ImGuiCol_TextLink]               = ImVec4(0.36f, 0.57f, 1.00f, 1.00f);    // Hyperlink color
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.50f, 0.66f, 1.00f, 1.00f);    // Text selection background
    colors[ImGuiCol_TreeLines]              = ImVec4(0.22f, 0.24f, 0.32f, 1.00f);    // Tree node lines

    // ---- Drag & Navigation ----
    colors[ImGuiCol_DragDropTarget]         = ImVec4(0.36f, 0.57f, 1.00f, 1.00f);    // Drag-drop target
    colors[ImGuiCol_NavCursor]              = ImVec4(0.49f, 0.66f, 1.00f, 1.00f);    // Navigation cursor
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);    // Windowing highlight
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.18f, 0.18f, 0.22f, 0.20f);    // Windowing dim background
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.10f, 0.12f, 0.18f, 0.45f);    // Modal window dim background
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
    const ImGuiID dockspace_id = setDockspaceID();

    static bool dockspaceInitialized = false;
    if (!dockspaceInitialized) {
        initializeDockspace(dockspace_id, viewport);
        dockspaceInitialized = true;
    }

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
    drawMainMenuBar();
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

void ImGuiContext::drawMainMenuBar() {
    const ImGuiIO& io = ImGui::GetIO();
    ImGui::PushStyleVarY(ImGuiStyleVar_FramePadding, 5.f);
    ImGui::PushFont(io.Fonts->Fonts[0]);
    if (ImGui::BeginMainMenuBar()) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 8));
        if (ImGui::BeginMenu((std::string(ICON_FA_FILE) + " File").c_str())) {
            if (ImGui::BeginMenu((std::string(ICON_FA_FILE_ARCHIVE_O) + " Save").c_str())) {
                if (ImGui::MenuItem((std::string(ICON_FA_CHECK) + " Save Current").c_str(), "Ctrl+S")) {
                    // TODO: Save scene
                }
                if (ImGui::MenuItem((std::string(ICON_FA_QUESTION) + " Save As...").c_str(), "Ctrl+Maj+S")) {
                    // TODO: Save scene as
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem((std::string(ICON_FA_FOLDER_OPEN) + " Open").c_str(), "Ctrl+O")) {
                // TODO: OpenScene(path)
            }
            if (ImGui::MenuItem((std::string(ICON_FA_CHECK) + " New").c_str(), "Ctrl+N")) {
                // TODO: New scene
            }
            if (ImGui::MenuItem((std::string(ICON_FA_CROSSHAIRS) + " Close").c_str(), "Ctrl+W")) {
                // TODO: Close scene
            }
            if (ImGui::MenuItem((std::string(ICON_FA_WINDOW_CLOSE) + " Exit").c_str() , "Ctrl+Q")) {
                // TODO: Close window
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

void ImGuiContext::moveSceneEntities() const {
    ImGui::Begin("Entities");
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
    ImGui::Begin("Scene specs", nullptr);
    ImGui::SliderFloat("Depth", &m_MaxDepth[m_camIdx], 1.0f, 100.f);
    ImGui::SliderFloat("FOV-Y", &m_FovY[m_camIdx], 30.f, 120.f);
    ImGui::SliderInt("Camera index", &m_camIdx, 0, 1);
    ImGui::End();
    moveSceneEntities();
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