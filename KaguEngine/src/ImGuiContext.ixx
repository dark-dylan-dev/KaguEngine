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

    void recreateSwapChain() const;
    void render(const Renderer& renderer);
    void onPresent(VkCommandBuffer commandBuffer);

    // Specs
    [[nodiscard]] float getDepth() const { return m_MaxDepth; }
    [[nodiscard]] float getFovY() const { return m_FovY; }
    [[nodiscard]] int getCamIdx() const { return m_camIdx; }

private:
    void setupContext() const;
    void setupConfigFlags();
    void setupStyle();
    void beginRender();
    void onRender(const Renderer& renderer);
    ImGuiViewport* setupViewport();
    ImGuiID setDockspaceID();
    void initializeDockspace(const ImGuiID& dockspace_id, const ImGuiViewport* viewport);
    void render3DScene(const Renderer& renderer);
    void moveSceneEntities();
    void renderSceneSpecs();
    void endRender();

    std::unique_ptr<DescriptorPool> &poolRef;
    std::vector<Entity> &viewsRef;
    Entity::Map &entitiesRef;
    Camera &cameraRef;
    Device &deviceRef;
    SwapChain &swapChainRef;
    Window &windowRef;

    // Specs
    float m_MaxDepth = 100.f;
    float m_FovY = 50.f;
    int m_camIdx = 0;
};

}