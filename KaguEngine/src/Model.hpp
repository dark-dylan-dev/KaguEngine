#pragma once

#include "Buffer.hpp"
#include "Device.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>

namespace KaguEngine {

class Model {
public:
    struct Vertex {
        glm::vec3 position{};
        glm::vec3 color{};
        glm::vec3 normal{};
        glm::vec2 texCoord{};

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

        bool operator==(const Vertex &other) const {
            return position == other.position &&
                   color == other.color &&
                   normal == other.normal &&
                   texCoord == other.texCoord;
        }
    };

    struct Builder {
        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices{};

        void loadModel(const std::string &filepath);
    };

    Model(Device &device, const Builder &builder);
    ~Model();

    // Non copyable
    Model(const Model &) = delete;
    Model &operator=(const Model &) = delete;

    static std::unique_ptr<Model> createModelFromFile(Device &device, const std::string &filepath);

    void bind(VkCommandBuffer commandBuffer) const;
    void draw(VkCommandBuffer commandBuffer) const;

private:
    void createVertexBuffers(const std::vector<Vertex> &vertices);
    void createIndexBuffers(const std::vector<uint32_t> &indices);

    Device& deviceRef;

    std::unique_ptr<Buffer> m_VertexBuffer;
    uint32_t m_VertexCount;

    bool m_HasIndexBuffer = false;
    std::unique_ptr<Buffer> m_IndexBuffer;
    uint32_t m_IndexCount;
};

} // Namespace KaguEngine
