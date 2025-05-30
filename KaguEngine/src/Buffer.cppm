module;

// libs
#include <vulkan/vulkan.h>

// std
#include <cassert>
import std.compat;

import Device;

export module Buffer;

export namespace KaguEngine {

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

// .cpp part

export namespace KaguEngine {

VkDeviceSize Buffer::getAlignment(const VkDeviceSize instanceSize, const VkDeviceSize minOffsetAlignment) {
    if (minOffsetAlignment > 0) {
        return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
    }
    return instanceSize;
}

Buffer::Buffer(Device &device,
    const VkDeviceSize instanceSize, const uint32_t instanceCount,
    const VkBufferUsageFlags usageFlags, const VkMemoryPropertyFlags memoryPropertyFlags,
    const VkDeviceSize minOffsetAlignment) :
    deviceRef{device},
    m_InstanceCount{instanceCount}, m_InstanceSize{instanceSize}, m_UsageFlags{usageFlags},
    m_MemoryPropertyFlags{memoryPropertyFlags}
{
    m_AlignmentSize = getAlignment(instanceSize, minOffsetAlignment);
    m_BufferSize = m_AlignmentSize * instanceCount;
    device.createBuffer(m_BufferSize, usageFlags, memoryPropertyFlags, m_Buffer, m_Memory);
}

Buffer::~Buffer() {
    unmap();
    vkDestroyBuffer(deviceRef.device(), m_Buffer, nullptr);
    vkFreeMemory(deviceRef.device(), m_Memory, nullptr);
}

VkResult Buffer::map(const VkDeviceSize size, const VkDeviceSize offset) {
    assert(m_Buffer && m_Memory && "Called map on buffer before create");
    return vkMapMemory(deviceRef.device(), m_Memory, offset, size, 0, &m_IsMapped);
}

void Buffer::unmap() {
    if (m_IsMapped) {
        vkUnmapMemory(deviceRef.device(), m_Memory);
        m_IsMapped = nullptr;
    }
}

void Buffer::writeToBuffer(const void *data, const VkDeviceSize size, const VkDeviceSize offset) const {
    assert(m_IsMapped && "Cannot copy to unmapped buffer");

    if (size == VK_WHOLE_SIZE) {
        memcpy(m_IsMapped, data, m_BufferSize);
    } else {
        auto memOffset = static_cast<char *>(m_IsMapped);
        memOffset += offset;
        memcpy(memOffset, data, size);
    }
}

VkResult Buffer::flush(const VkDeviceSize size, const VkDeviceSize offset) const {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = m_Memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkFlushMappedMemoryRanges(deviceRef.device(), 1, &mappedRange);
}

VkResult Buffer::invalidate(const VkDeviceSize size, const VkDeviceSize offset) const {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = m_Memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkInvalidateMappedMemoryRanges(deviceRef.device(), 1, &mappedRange);
}

VkDescriptorBufferInfo Buffer::descriptorInfo(const VkDeviceSize size, const VkDeviceSize offset) const {
    return VkDescriptorBufferInfo{
        m_Buffer,
        offset,
        size,
    };
}

void Buffer::writeToIndex(const void *data, const int index) const {
    writeToBuffer(data, m_InstanceSize, index * m_AlignmentSize);
}

VkResult Buffer::flushIndex(const int index) const {
    return flush(m_AlignmentSize, index * m_AlignmentSize);
}

VkDescriptorBufferInfo Buffer::descriptorInfoForIndex(const int index) const {
    return descriptorInfo(m_AlignmentSize, index * m_AlignmentSize);
}

VkResult Buffer::invalidateIndex(const int index) const {
    return invalidate(m_AlignmentSize, index * m_AlignmentSize);
}

} // Namespace KaguEngine