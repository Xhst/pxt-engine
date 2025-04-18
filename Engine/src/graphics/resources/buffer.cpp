#include "graphics/resources/buffer.hpp"

#include "core/error_handling.hpp"
#include <cstring>
#include <stdexcept>

namespace PXTEngine {

    VkDeviceSize Buffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
        if (minOffsetAlignment > 0) {
            return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
        }
        return instanceSize;
    }

    Buffer::Buffer(Context& context,
        VkDeviceSize instanceSize,
        uint32_t instanceCount,
        VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags memoryPropertyFlags,
        VkDeviceSize minOffsetAlignment)
        : m_context{context},
          m_instanceSize{instanceSize},
          m_instanceCount{instanceCount},
          m_usageFlags{usageFlags},
          m_memoryPropertyFlags{memoryPropertyFlags} {
        m_alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
        m_bufferSize = m_alignmentSize * instanceCount;
        context.createBuffer(m_bufferSize, usageFlags, memoryPropertyFlags, m_buffer, m_memory);
    }

    Buffer::~Buffer() {
        unmap();
        vkDestroyBuffer(m_context.getDevice(), m_buffer, nullptr);
        vkFreeMemory(m_context.getDevice(), m_memory, nullptr);
    }

    VkResult Buffer::map(VkDeviceSize size, VkDeviceSize offset) {
        PXT_ASSERT(m_buffer && m_memory, "Called map on buffer before create");

        return vkMapMemory(m_context.getDevice(), m_memory, offset, size, 0, &m_mapped);
    }

    void Buffer::unmap() {
        if (m_mapped) {
            vkUnmapMemory(m_context.getDevice(), m_memory);
            m_mapped = nullptr;
        }
    }

	// TODO: add "if NDEBUG ... we avoid checks"
	void Buffer::writeToBuffer(void* data, VkDeviceSize size, VkDeviceSize offset) {
        if (!m_mapped) {
			throw std::runtime_error("Cannot write to buffer: memory is not mapped.");
		}

		if (!data) {
			throw std::runtime_error("Cannot write to buffer: data pointer is null.");
		}

		VkDeviceSize writeSize = (size == VK_WHOLE_SIZE) ? m_bufferSize : size;

		if (offset + writeSize > m_bufferSize) {
			throw std::out_of_range("Buffer write exceeds buffer bounds (offset + size > buffer size).");
		}

		char* memOffset = reinterpret_cast<char*>(m_mapped) + offset;
		memcpy(memOffset, data, writeSize);
	}

    VkResult Buffer::flush(VkDeviceSize size, VkDeviceSize offset) {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = m_memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkFlushMappedMemoryRanges(m_context.getDevice(), 1, &mappedRange);
    }

    VkResult Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = m_memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkInvalidateMappedMemoryRanges(m_context.getDevice(), 1, &mappedRange);
    }

    VkDescriptorBufferInfo Buffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
        return VkDescriptorBufferInfo{
            m_buffer,
            offset,
            size,
        };
    }

    void Buffer::writeToIndex(void *data, int index) {
        writeToBuffer(data, m_instanceSize, index * m_alignmentSize);
    }

    VkResult Buffer::flushIndex(int index) { return flush(m_alignmentSize, index * m_alignmentSize); }

    VkDescriptorBufferInfo Buffer::descriptorInfoForIndex(int index) {
        return descriptorInfo(m_alignmentSize, index * m_alignmentSize);
    }

    VkResult Buffer::invalidateIndex(int index) {
        return invalidate(m_alignmentSize, index * m_alignmentSize);
    }

}