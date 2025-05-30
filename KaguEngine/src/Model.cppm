module;

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/glm.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <vulkan/vulkan.h>

// std
#include <memory>
#include <unordered_map>
#include <vector>

export module Model;

import Buffer;
import Device;
import Utils;
import Texture;

export namespace KaguEngine {

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

// .cpp part

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

export namespace KaguEngine {

Model::Model(Device& device, const Builder& builder) : deviceRef{device} {
    createVertexBuffers(builder.vertices);
    createIndexBuffers(builder.indices);
}

Model::~Model() = default;

std::unique_ptr<Model> Model::createModelFromFile(Device& device, const std::string& filepath) {
    Builder builder{};
    builder.loadModel(/*ENGINE_DIR + */filepath);
    return std::make_unique<Model>(device, builder);
}

void Model::createVertexBuffers(const std::vector<Vertex> &vertices) {
    m_VertexCount = static_cast<uint32_t>(vertices.size());
    assert(m_VertexCount >= 3 && "Vertex count must be at least 3");

    const VkDeviceSize bufferSize = sizeof(vertices[0]) * m_VertexCount;
    uint32_t vertexSize = sizeof(vertices[0]);

    Buffer stagingBuffer{
            deviceRef,
            vertexSize,
            m_VertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer(vertices.data());

    m_VertexBuffer = std::make_unique<Buffer>(deviceRef, vertexSize, m_VertexCount,
                                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    deviceRef.copyBuffer(stagingBuffer.getBuffer(), m_VertexBuffer->getBuffer(), bufferSize);
}

void Model::createIndexBuffers(const std::vector<uint32_t> &indices) {
    m_IndexCount = static_cast<uint32_t>(indices.size());
    m_HasIndexBuffer = m_IndexCount > 0;

    if (!m_HasIndexBuffer) {
        return;
    }

    const VkDeviceSize bufferSize = sizeof(indices[0]) * m_IndexCount;
    uint32_t indexSize = sizeof(indices[0]);

    Buffer stagingBuffer{
            deviceRef,
            indexSize,
            m_IndexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer(indices.data());

    m_IndexBuffer = std::make_unique<Buffer>(deviceRef, indexSize, m_IndexCount,
                                           VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    deviceRef.copyBuffer(stagingBuffer.getBuffer(), m_IndexBuffer->getBuffer(), bufferSize);
}

void Model::draw(const VkCommandBuffer commandBuffer) const {
    if (m_HasIndexBuffer) {
        vkCmdDrawIndexed(commandBuffer, m_IndexCount, 1, 0, 0, 0);
    } else {
        vkCmdDraw(commandBuffer, m_VertexCount, 1, 0, 0);
    }
}

void Model::bind(const VkCommandBuffer commandBuffer) const {
    const VkBuffer buffers[] = {m_VertexBuffer->getBuffer()};
    constexpr VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

    if (m_HasIndexBuffer) {
        vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }
}

std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions() {
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions() {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

    attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
    attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
    attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
    attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(Vertex, texCoord)});

    return attributeDescriptions;
}

void Model::Builder::loadModel(const std::string &filepath) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
        throw std::runtime_error(warn + err);
    }

    vertices.clear();
    indices.clear();

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    for (const auto &shape: shapes) {
        for (const auto &[vertex_index, normal_index, texcoord_index]: shape.mesh.indices) {
            Vertex vertex{};

            if (vertex_index >= 0) {
                vertex.position = {
                        attrib.vertices[3 * vertex_index + 0],
                        attrib.vertices[3 * vertex_index + 1],
                        attrib.vertices[3 * vertex_index + 2],
                };

                vertex.color = {
                        attrib.colors[3 * vertex_index + 0],
                        attrib.colors[3 * vertex_index + 1],
                        attrib.colors[3 * vertex_index + 2],
                };
            }

            if (normal_index >= 0) {
                vertex.normal = {
                        attrib.normals[3 * normal_index + 0],
                        attrib.normals[3 * normal_index + 1],
                        attrib.normals[3 * normal_index + 2],
                };
            }

            // Flips the texture's uv coordinates
            if (texcoord_index >= 0) {
                vertex.texCoord = {
                    attrib.texcoords[2 * texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * texcoord_index + 1]
                };
            }

            if (!uniqueVertices.contains(vertex)) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }
            indices.push_back(uniqueVertices[vertex]);
        }
    }
}

} // Namespace KaguEngine
