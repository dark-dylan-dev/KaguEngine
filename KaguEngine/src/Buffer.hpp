#pragma once

#include "Device.hpp"

namespace KaguEngine {

class Buffer {
public:
    Buffer(Device& device, VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags,
           VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment = 1);
    ~Buffer();

    // Non copyable
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void unmap();

    void writeToBuffer(const void *data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
    [[nodiscard]] VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
    [[nodiscard]] VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
    [[nodiscard]] VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;

    void writeToIndex(const void *data, int index) const;
    [[nodiscard]] VkResult flushIndex(int index) const;
    [[nodiscard]] VkDescriptorBufferInfo descriptorInfoForIndex(int index) const;
    [[nodiscard]] VkResult invalidateIndex(int index) const;

    [[nodiscard]] VkBuffer getBuffer()                           const { return m_Buffer; }
    [[nodiscard]] void* getMappedMemory()                        const { return m_IsMapped; }
    [[nodiscard]] uint32_t getInstanceCount()                    const { return m_InstanceCount; }
    [[nodiscard]] VkDeviceSize getInstanceSize()                 const { return m_InstanceSize; }
    [[nodiscard]] VkDeviceSize getAlignmentSize()                const { return m_InstanceSize; }
    [[nodiscard]] VkBufferUsageFlags getUsageFlags()             const { return m_UsageFlags; }
    [[nodiscard]] VkMemoryPropertyFlags getMemoryPropertyFlags() const { return m_MemoryPropertyFlags; }
    [[nodiscard]] VkDeviceSize getBufferSize()                   const { return m_BufferSize; }

private:
    static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

    Device& deviceRef;
    void* m_IsMapped = nullptr;
    VkBuffer m_Buffer = VK_NULL_HANDLE;
    VkDeviceMemory m_Memory = VK_NULL_HANDLE;

    VkDeviceSize m_BufferSize;
    uint32_t m_InstanceCount;
    VkDeviceSize m_InstanceSize;
    VkDeviceSize m_AlignmentSize;
    VkBufferUsageFlags m_UsageFlags;
    VkMemoryPropertyFlags m_MemoryPropertyFlags;
};

} // Namespace KaguEngine
