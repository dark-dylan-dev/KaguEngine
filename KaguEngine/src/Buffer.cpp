#include "Buffer.hpp"

// std
#include <cassert>
#include <cstring>

namespace KaguEngine {

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
    instanceCount{instanceCount}, instanceSize{instanceSize}, usageFlags{usageFlags},
    memoryPropertyFlags{memoryPropertyFlags}
{
    alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
    bufferSize = alignmentSize * instanceCount;
    device.createBuffer(bufferSize, usageFlags, memoryPropertyFlags, buffer, memory);
}

Buffer::~Buffer() {
    unmap();
    vkDestroyBuffer(deviceRef.device(), buffer, nullptr);
    vkFreeMemory(deviceRef.device(), memory, nullptr);
}

VkResult Buffer::map(const VkDeviceSize size, const VkDeviceSize offset) {
    assert(buffer && memory && "Called map on buffer before create");
    return vkMapMemory(deviceRef.device(), memory, offset, size, 0, &mapped);
}

void Buffer::unmap() {
    if (mapped) {
        vkUnmapMemory(deviceRef.device(), memory);
        mapped = nullptr;
    }
}

void Buffer::writeToBuffer(const void *data, const VkDeviceSize size, const VkDeviceSize offset) const {
    assert(mapped && "Cannot copy to unmapped buffer");

    if (size == VK_WHOLE_SIZE) {
        memcpy(mapped, data, bufferSize);
    } else {
        auto memOffset = static_cast<char *>(mapped);
        memOffset += offset;
        memcpy(memOffset, data, size);
    }
}

VkResult Buffer::flush(const VkDeviceSize size, const VkDeviceSize offset) const {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkFlushMappedMemoryRanges(deviceRef.device(), 1, &mappedRange);
}

VkResult Buffer::invalidate(const VkDeviceSize size, const VkDeviceSize offset) const {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkInvalidateMappedMemoryRanges(deviceRef.device(), 1, &mappedRange);
}

VkDescriptorBufferInfo Buffer::descriptorInfo(const VkDeviceSize size, const VkDeviceSize offset) const {
    return VkDescriptorBufferInfo{
        buffer,
        offset,
        size,
    };
}

void Buffer::writeToIndex(const void *data, const int index) const {
    writeToBuffer(data, instanceSize, index * alignmentSize);
}

VkResult Buffer::flushIndex(const int index) const {
    return flush(alignmentSize, index * alignmentSize);
}

VkDescriptorBufferInfo Buffer::descriptorInfoForIndex(const int index) const {
    return descriptorInfo(alignmentSize, index * alignmentSize);
}

VkResult Buffer::invalidateIndex(const int index) const {
    return invalidate(alignmentSize, index * alignmentSize);
}

} // Namespace KaguEngine
