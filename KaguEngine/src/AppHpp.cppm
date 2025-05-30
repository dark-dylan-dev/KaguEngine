module;

// libs
#include <vulkan/vulkan.h>

// std
#include <memory>

export module App:Hpp;

export import Descriptor;
export import Device;
export import Entity;
export import Renderer;
export import Window;

export namespace KaguEngine {

class App {
public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

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
            .build();
        loadGameObjects();
    };
    ~App() = default;

    App(const App &) = delete;
    App &operator=(const App &) = delete;

    void run();

private:
    void loadGameObjects();

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
