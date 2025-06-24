module;

// libs
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
        Entity::Map& sceneEntities, std::vector<Entity>& views, Camera& camera
    );
    ~ImGuiContext();

    static void recreateSwapChain();
    void render(const Renderer& renderer);

    static void onPresent(VkCommandBuffer commandBuffer);

    // Specs
    [[nodiscard]] float getDepth() const { return m_MaxDepth[m_camIdx]; }
    [[nodiscard]] float getFovY() const { return m_FovY[m_camIdx]; }
    [[nodiscard]] int getCamIdx() const { return m_camIdx; }

private:
    // Setup part
    static void setupConfigFlags();
    static void setupStyle();
    static void setAppTheme();
    static void setStyleVars();
    void setupContext() const;
    static void initializeDockspace(const ImGuiID& dockspace_id, const ImGuiViewport* viewport);
    // Rendering part
    static void beginRender();
    void onRender(const Renderer& renderer);
    static ImGuiViewport* setupViewport();
    static ImGuiID setDockspaceID();
    static void drawMainMenuBar();
    static void render3DScene(const Renderer& renderer);
    void moveSceneEntities() const;
    void renderSceneSpecs();
    static void endRender();

    std::unique_ptr<DescriptorPool> &poolRef;
    std::vector<Entity> &viewsRef;
    Entity::Map &entitiesRef;
    Camera &cameraRef;
    Device &deviceRef;
    SwapChain &swapChainRef;
    Window &windowRef;

    // Specs
    std::vector<float> m_MaxDepth = { 100.f, 100.f };
    std::vector<float> m_FovY = { 50.f, 50.f };
    int m_camIdx = 0;
};

}