module;

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

module App;

// std
import std;

import KaguEngine.Buffer;
import KaguEngine.Camera;
import KaguEngine.Descriptor;
import KaguEngine.Device;
import KaguEngine.Entity;
import KaguEngine.FrameInfo;
import KaguEngine.ImGuiContext;
import KaguEngine.Model;
import KaguEngine.MovementController;
import KaguEngine.Renderer;
import KaguEngine.System.PointLight;
import KaguEngine.System.Render;
import KaguEngine.SwapChain;
import KaguEngine.Texture;
import KaguEngine.Window;

namespace KaguEngine {

void App::run() {
    std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (auto& uboBuffer : uboBuffers) {
        uboBuffer = std::make_unique<Buffer>(m_Device, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        uboBuffer->map();
    }

    std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (unsigned int i = 0; i < globalDescriptorSets.size(); i++) {
        auto bufferInfo = uboBuffers[i]->descriptorInfo();
        DescriptorWriter(*m_GlobalSetLayout, *m_DescriptorPool)
            .writeBuffer(0, &bufferInfo)
            .build(globalDescriptorSets[i]);
    }

    RenderSystem renderSystem{
        m_Device,
        m_Renderer.getSwapChainRenderPass(),
        m_GlobalSetLayout->getDescriptorSetLayout(),    // set = 0 (UBO)
        m_MaterialSetLayout->getDescriptorSetLayout()   // set = 1 (textures)
    };
    PointLightSystem pointLightSystem{
        m_Device,
        m_Renderer.getSwapChainRenderPass(),
        m_GlobalSetLayout->getDescriptorSetLayout()
    };
    ImGuiContext imGuiContext(m_Window, *m_Renderer.getSwapChain(), m_Device, m_DescriptorPool);
    Camera camera{};

    auto viewerObject = Entity::createEntity();
    viewerObject.transform.translation = {0.f, -0.5f, -3.f};

    auto currentTime = std::chrono::high_resolution_clock::now();
    while (!m_Window.shouldClose()) {
        KeyboardMovementController cameraController{};
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        cameraController.moveInPlaneXZ(m_Window.getGLFWwindow(), frameTime, viewerObject);
        camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

        float aspect = m_Renderer.getAspectRatio();
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

        if (auto commandBuffer = m_Renderer.beginFrame()) {
            int frameIndex = m_Renderer.getFrameIndex();
            FrameInfo frameInfo{
                frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex], m_SceneEntities
            };

            // update
            GlobalUbo ubo{};
            ubo.projection = camera.getProjection();
            ubo.view = camera.getView();
            ubo.inverseView = camera.getInverseView();
            PointLightSystem::update(frameInfo, ubo);
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            if(uboBuffers[frameIndex]->flush() != VK_SUCCESS)
                throw std::runtime_error("Couldn't flush the ubo for one frame!");

            // render
            m_Renderer.beginSwapChainRenderPass(commandBuffer);
            if (m_Renderer.isSwapChainRecreated()) {
                imGuiContext.recreateSwapChain();
                m_Renderer.setSwapChainRecreated();
            }

            // order here matters
            renderSystem.renderGameObjects(frameInfo);
            pointLightSystem.render(frameInfo);

            // ImGui
            imGuiContext.render(commandBuffer);

            m_Renderer.endSwapChainRenderPass(commandBuffer);
            if (m_Renderer.isSwapChainRecreated()) {
                imGuiContext.recreateSwapChain();
                m_Renderer.setSwapChainRecreated();
            }
            m_Renderer.endFrame();
        }
    }

    vkDeviceWaitIdle(m_Device.device());
}

void App::loadGameObjects() {
    std::shared_ptr<Model> loadedModel;

    // Obamium
    auto obamiumTexture = Texture::createTextureFromFile(m_Device, *m_Renderer.getSwapChain(),
        "assets/textures/obamium_texture.png", m_MaterialSetLayout->getDescriptorSetLayout(),
        m_DescriptorPool->getDescriptorPool()
    );
    loadedModel = Model::createModelFromFile(m_Device, "assets/models/obamium_model.obj");

    auto centralObamium = Entity::createEntity();
    centralObamium.model = loadedModel;
    centralObamium.texture = std::move(obamiumTexture);
    centralObamium.material = centralObamium.texture->getMaterial();
    centralObamium.transform.translation = {0.0f, 0.0f, 0.0f};
    centralObamium.transform.scale = {1.f, 1.f, 1.f};
    centralObamium.transform.rotation = {0.f, 0.f, 3.14159265f};
    m_SceneEntities.emplace(centralObamium.getId(), std::move(centralObamium));

    // Viking room
    auto vikingRoomTexture = Texture::createTextureFromFile(m_Device, *m_Renderer.getSwapChain(),
        "assets/textures/viking_room.png", m_MaterialSetLayout->getDescriptorSetLayout(),
        m_DescriptorPool->getDescriptorPool()
    );
    loadedModel = Model::createModelFromFile(m_Device, "assets/models/viking_room.obj");

    auto vikingRoom = Entity::createEntity();
    vikingRoom.model = loadedModel;
    vikingRoom.texture = std::move(vikingRoomTexture);
    vikingRoom.material = vikingRoom.texture->getMaterial();
    vikingRoom.transform.translation = {2.f, 0.f, 2.f};
    vikingRoom.transform.scale = {1.f, 1.f, 1.f};
    vikingRoom.transform.rotation = {3.14159265f / 2.f, 0.f, 3.14159265f};
    m_SceneEntities.emplace(vikingRoom.getId(), std::move(vikingRoom));

    // Floor
    auto dummyTexture = Texture::makeDummyTexture(
        m_Device,
        *m_Renderer.getSwapChain(),
        m_MaterialSetLayout->getDescriptorSetLayout(),
        m_DescriptorPool->getDescriptorPool()
    );
    loadedModel = Model::createModelFromFile(m_Device, "assets/models/base.obj");
    auto floor = Entity::createEntity();
    floor.model = loadedModel;
    floor.texture = std::move(dummyTexture);
    floor.material = floor.texture->getMaterial();
    floor.transform.translation = {0.f, 0.5f, 0.f};
    floor.transform.scale = {1.f, 1.f, 1.f};
    m_SceneEntities.emplace(floor.getId(), std::move(floor));

    std::vector<glm::vec3> lightColors{
        {1.f, .1f, .1f},
        {.1f, .1f, 1.f},
        {.1f, 1.f, .1f},
        {1.f, 1.f, .1f},
        {.1f, 1.f, 1.f},
        {1.f, 1.f, 1.f}
    };

    for (int i = 0; i < lightColors.size(); i++) {
        auto pointLight = Entity::makePointLight(0.2f);
        pointLight.color = lightColors[i];
        auto rotateLight = glm::rotate(
            glm::mat4(1.f),
            static_cast<float>(i) * glm::two_pi<float>() / static_cast<float>(lightColors.size()),
            {0.f, -1.f, 0.f});
        pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
        m_SceneEntities.emplace(pointLight.getId(), std::move(pointLight));
    }
}

} // Namespace KaguEngine