module;

// libs
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

export module KaguEngine.Model;

// std
import std;

import KaguEngine.Buffer;
import KaguEngine.Device;
import KaguEngine.Utils;

export namespace KaguEngine {

class Model {
public:
    struct Vertex {
        glm::vec3 position{};
        glm::vec3 color{};
        glm::vec3 normal{};
        glm::vec2 texCoord{};

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(bool isTextured);

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

// Vertices hashing, defined in Utils.ixx
export {
namespace std {
template<>
struct hash<KaguEngine::Model::Vertex> {
    size_t operator()(KaguEngine::Model::Vertex const &vertex) const noexcept {
        size_t seed = 0;
        KaguEngine::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.texCoord);
        return seed;
    }
};
}
}
