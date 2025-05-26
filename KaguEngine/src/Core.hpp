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

class Core {
public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    Core();
    ~Core();

    Core(const Core &) = delete;
    Core &operator=(const Core &) = delete;

    void run();

private:
    void loadGameObjects();

    Window m_Window{WIDTH, HEIGHT, "Kagu Engine"};
    Device m_Device{m_Window};
    Renderer m_Renderer{m_Window, m_Device};

    // note: order of declarations matters
    std::unique_ptr<DescriptorSetLayout> m_GlobalSetLayout{};
    std::unique_ptr<DescriptorSetLayout> m_MaterialSetLayout{};
    std::unique_ptr<DescriptorPool> m_DescriptorPool{};
    Entity::Map sceneEntities;
};

} // Namespace KaguEngine
