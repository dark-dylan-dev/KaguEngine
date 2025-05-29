#pragma once

#include "Descriptor.hpp"
#include "Device.hpp"
#include "Entity.hpp"
#include "Renderer.hpp"
#include "Window.hpp"

// std
#include <memory>
#include <vector>
#include <iostream>

namespace KaguEngine {

class App {
public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    App();
    ~App();

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
