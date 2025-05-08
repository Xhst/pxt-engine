#pragma once

#include "graphics/context/context.hpp"

namespace PXTEngine {

    /**
     * @brief A class representing a Vulkan buffer.
     *
     * This class encapsulates the creation and management of Vulkan buffers, providing
     * functionalities for mapping, unmapping, writing data, flushing, and retrieving
     * descriptor information. It also supports writing and managing data for individual
     * instances within the buffer.
     */
    class VulkanBuffer {
        public:
        /**
         * @brief Constructor for the Buffer class.
         *
         * @param context The Vulkan context used to create the buffer.
         * @param instanceSize The size of each instance within the buffer.
         * @param instanceCount The number of instances in the buffer.
         * @param usageFlags Vulkan buffer usage flags.
         * @param memoryPropertyFlags Vulkan memory property flags.
         * @param minOffsetAlignment Minimum offset alignment for the buffer.
         */
        VulkanBuffer(Context& context, VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags,
               VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment = 1);

        /**
         * @brief Destructor for the Buffer class.
         *
         * Releases the Vulkan buffer and memory.
         */
        ~VulkanBuffer();

        /**
         * @brief Deleted copy constructor.
         */
        VulkanBuffer(const VulkanBuffer&) = delete;

        /**
         * @brief Deleted copy assignment operator.
         */
        VulkanBuffer& operator=(const VulkanBuffer&) = delete;

        /**
         * @brief Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
         *
         * @param size (Optional) The size of the memory to map. Defaults to the entire buffer size.
         *             Pass VK_WHOLE_SIZE to map the complete buffer range.
         * @param offset (Optional) The offset from the beginning of the buffer to start mapping.
         * @return VkResult of the buffer mapping call.
         */
        VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        /**
         * @brief Unmap a mapped memory range
         * 
         * @note Does not return a result as vkUnmapMemory can't fail
         */
        void unmap();

        /**
         * @brief Copies the specified data to the mapped buffer. Default value writes whole buffer range.
         *
         * @param data Pointer to the data to copy.
         * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer range.
         * @param offset (Optional) Byte offset from beginning of mapped region
         */
        void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        /**
         * @brief Flushes the mapped memory range of the buffer to make it visible to the device.
         *
         * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the complete buffer range.
         * @param offset (Optional) The offset from the beginning of the buffer to flush.
         * @return VkResult of the flush call.
         */
        VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        /**
         * @brief Returns the descriptor buffer info for the buffer.
         *
         * @param size (Optional) Size of the memory range of the descriptor
         * @param offset (Optional) Byte offset from beginning
         *
         * @return VkDescriptorBufferInfo of specified offset and range
         */
        VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        /**
         * @brief Invalidate a memory range of the buffer to make it visible to the host
         *
         * @note Only required for non-coherent memory
         *
         * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate
         * the complete buffer range.
         * @param offset (Optional) Byte offset from beginning
         *
         * @return VkResult of the invalidate call
         */
        VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        /**
         * @brief Writes data to a specific instance within the buffer.
         * Copies "instanceSize" bytes of data to the mapped buffer at an offset of index * alignmentSize
         *
         * @param data The data to write.
         * @param index The index of the instance to write to.
         */
        
        void writeToIndex(void* data, int index);

        /**
         * @brief Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
         *
         * @param index Used in offset calculation
         *
         */
        VkResult flushIndex(int index);

        /**
         * @brief Create a buffer info descriptor.
         *
         * @param index Specifies the region given by index * alignmentSize.
         * @return VkDescriptorBufferInfo for instance at index.
         */
        VkDescriptorBufferInfo descriptorInfoForIndex(int index);

        /**
         * @brief Invalidate a memory range of the buffer to make it visible to the host
         *
         * @note Only required for non-coherent memory
         *
         * @param index Specifies the region to invalidate: index * alignmentSize
         *
         * @return VkResult of the invalidate call
         */
        VkResult invalidateIndex(int index);

        /**
         * @brief Returns the Vulkan buffer handle.
         *
         * @return The Vulkan buffer handle.
         */
        VkBuffer getBuffer() const { return m_buffer; }

        /**
         * @brief Returns the mapped memory pointer.
         *
         * @return The mapped memory pointer.
         */
        void* getMappedMemory() const { return m_mapped; }

        /**
         * @brief Returns the number of instances in the buffer.
         *
         * @return The number of instances.
         */
        uint32_t getInstanceCount() const { return m_instanceCount; }

        /**
         * @brief Returns the size of each instance.
         *
         * @return The instance size.
         */
        VkDeviceSize getInstanceSize() const { return m_instanceSize; }

        /**
         * @brief Returns the alignment size for instances.
         *
         * @return The alignment size.
         */
        VkDeviceSize getAlignmentSize() const { return m_instanceSize; }

        /**
         * @brief Returns the buffer usage flags.
         *
         * @return The buffer usage flags.
         */
        VkBufferUsageFlags getUsageFlags() const { return m_usageFlags; }

        /**
         * @brief Returns the memory property flags.
         *
         * @return The memory property flags.
         */
        VkMemoryPropertyFlags getMemoryPropertyFlags() const { return m_memoryPropertyFlags; }

        /**
         * @brief Returns the total buffer size.
         *
         * @return The buffer size.
         */
        VkDeviceSize getBufferSize() const { return m_bufferSize; }

        private:
        /**
         * @brief Calculates the aligned instance size.
         * 
         * Returns the minimum instance size required to be compatible with devices minOffsetAlignment
         * (rounds up instanceSize to the next multiple of minOffsetAlignment, if 1 -> returns instanceSize
         *
         * @param instanceSize The original instance size.
         * @param minOffsetAlignment The minimum required alignment, in bytes, for the offset member 
         *                           (eg. minUniformBufferOffsetAlignment)
         * 
         * @return VkResult of the buffer mapping call
         */
        static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

		VkDeviceAddress getDeviceAddress() const;

        Context& m_context;
        void* m_mapped = nullptr;
        VkBuffer m_buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_memory = VK_NULL_HANDLE;

        VkDeviceSize m_bufferSize;
        uint32_t m_instanceCount;
        VkDeviceSize m_instanceSize;
        VkDeviceSize m_alignmentSize;
        VkBufferUsageFlags m_usageFlags;
        VkMemoryPropertyFlags m_memoryPropertyFlags;
    };

}