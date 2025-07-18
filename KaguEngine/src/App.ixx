module;

// libs
#include <vulkan/vulkan.h>

export module App;

// std - exported for convenience in main.cpp
export import std;

import KaguEngine.Descriptor;
import KaguEngine.Device;
import KaguEngine.Entity;
import KaguEngine.Renderer;
import KaguEngine.Window;

export namespace KaguEngine {

class App {
public:
    static constexpr int WIDTH = 1600;
    static constexpr int HEIGHT = 900;

    App() {
        m_GlobalSetLayout = DescriptorSetLayout::Builder(m_Device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();
        m_MaterialSetLayout = DescriptorSetLayout::Builder(m_Device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();
        m_DescriptorPool = DescriptorPool::Builder(m_Device)
            .setMaxSets(1000) // Arbitrary value
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 1000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000)
            .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
            .build();
        loadGameObjects();
    };
    ~App() {
        std::erase_if(m_SceneEntities, [](const auto&) { return true; });
    }

    App(const App &) = delete;
    App &operator=(const App &) = delete;

    void run();

private:
    void loadGameObjects();
    bool m_IsRunning = true;

    Window m_Window{WIDTH, HEIGHT, "Kagu Engine"};
    Device m_Device{m_Window};
    Renderer m_Renderer{m_Window, m_Device};

    // note: order of declarations matters
    //  - class destroyed from bottom to top
    //  - entities -> pool -> material set -> global set
    std::unique_ptr<DescriptorSetLayout> m_GlobalSetLayout{};
    std::unique_ptr<DescriptorSetLayout> m_MaterialSetLayout{};
    std::unique_ptr<DescriptorPool> m_DescriptorPool{};
    Entity::Map m_SceneEntities;
};

} // Namespace KaguEngine