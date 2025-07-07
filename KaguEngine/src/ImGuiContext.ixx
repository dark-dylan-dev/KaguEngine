module;

// libs
#include <glm/glm.hpp>
#include <imgui.h>
#include <vulkan/vulkan.h>

export module KaguEngine.ImGuiContext;

// std
import std;

import KaguEngine.Camera;
import KaguEngine.Descriptor;
import KaguEngine.Device;
import KaguEngine.Entity;
import KaguEngine.SwapChain;
import KaguEngine.Renderer;
import KaguEngine.Window;

export namespace KaguEngine {

class ImGuiContext {

public:
    ImGuiContext(
        Window &window, SwapChain &swapChain, Device &device,
        std::unique_ptr<DescriptorPool> &pool,
        Entity::Map& sceneEntities, std::vector<Entity>& views, Camera& camera,
        glm::vec4& ambientLight, glm::vec4& clearColor
    );
    ~ImGuiContext();

    static void recreateSwapChain();
    void render(const Renderer& renderer);

    static void onPresent(VkCommandBuffer commandBuffer);

    // Specs
    [[nodiscard]] float getDepth()                  const { return m_MaxDepth[m_CamIdx]; }
    [[nodiscard]] float getFovY()                   const { return glm::radians(m_FovY[m_CamIdx]); } // Radians
    [[nodiscard]] float getFovX()                   const { return glm::radians(m_FovX[m_CamIdx]); } // Radians
    [[nodiscard]] Entity& getView()                 const { return viewsRef[m_CamIdx]; }
    [[nodiscard]] int getCamIdx()                   const { return m_CamIdx; }
    [[nodiscard]] glm::vec4& getAmbientLightColor() const { return ambientLightColorRef; }
    [[nodiscard]] glm::vec4& getClearColor()        const { return clearColorRef; }
    [[nodiscard]] bool isRunning()                  const { return m_IsRunning; }

    // Console
    void addLog(const char* msg) { m_Items.emplace_back(msg); }

private:
    // --- Setup ---
    void setupContext() const;
    static void setupConfigFlags();
    static void setupStyle();
    static void setAppTheme();
    static void setStyleVars();
    static void initializeDockspace(const ImGuiID& dockspace_id, const ImGuiViewport* viewport);

    // --- Main Rendering Flow ---
    static void beginRender();
    void onRender(const Renderer& renderer);
    static void endRender();

    // --- UI Panel Rendering ---
    static ImGuiViewport* setupViewport();
    ImGuiID getDockspaceID();
    void drawMainMenuBar();
    void render3DScene(const Renderer& renderer) const;
    void renderSceneHierarchyPanel();
    void renderPropertiesPanel();
    void renderVisualsPanel();
    void renderConsole();

    // --- State ---
    bool m_IsRunning = true;
    Entity::id_t m_SelectedEntityID = std::numeric_limits<Entity::id_t>::max();

    // --- Lvalue References to Engine State ---
    glm::vec4& ambientLightColorRef;
    glm::vec4& clearColorRef;
    std::unique_ptr<DescriptorPool> &poolRef;
    std::vector<Entity> &viewsRef;
    Entity::Map &entitiesRef;
    Camera &cameraRef;
    Device &deviceRef;
    SwapChain &swapChainRef;
    Window &windowRef;

    // --- UI/Editor State ---
    std::vector<float> m_MaxDepth = { 100.f, 100.f };
    std::vector<float> m_FovY = { 70.f, 70.f };
    std::vector<float> m_FovX = { 90.f, 90.f };
    int m_CamIdx = 0;

    // --- Console State ---
    bool m_ConsoleOpened = true;
    char m_InputBuffer[256];
    std::vector<std::string> m_Items = { "Welcome to Kagu Engine!", "[Info] Still in development!", "[Hint] Select an entity in the hierarchy to see its properties." };
};

}