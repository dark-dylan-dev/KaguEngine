module;

#include <vulkan/vulkan.h>

export module KaguEngine.ImGuiContext;

// std
import std;

import KaguEngine.Descriptor;
import KaguEngine.Device;
import KaguEngine.SwapChain;
import KaguEngine.Renderer;
import KaguEngine.Window;

export namespace KaguEngine {

class ImGuiContext {

public:
    ImGuiContext(Window &window, SwapChain &swapChain, Device &device, std::unique_ptr<DescriptorPool> &pool);
    ~ImGuiContext();

    void recreateSwapChain() const;
    void render(Renderer& renderer);
    void onPresent(VkCommandBuffer commandBuffer);

private:
    void setupContext() const;
    void setupConfigFlags();
    void setupStyle();
    void beginRender();
    void onRender(Renderer& renderer);
    void endRender();

    std::unique_ptr<DescriptorPool> &poolRef;
    Device &deviceRef;
    SwapChain &swapChainRef;
    Window &windowRef;
};

}