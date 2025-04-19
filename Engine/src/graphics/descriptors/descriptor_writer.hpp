#pragma once

#include "core/error_handling.hpp"
#include "graphics/descriptors/descriptor_set_layout.hpp"
#include "graphics/descriptors/descriptor_pool.hpp"


namespace PXTEngine {
    class DescriptorWriter {
    public:

        DescriptorWriter(Context& context, DescriptorSetLayout& setLayout);

        /**
         * @brief Writes a single buffer descriptor to the specified binding.
         * 
         * @param binding The binding index.
         * @param bufferInfo Pointer to the buffer descriptor info.
         * 
         * @return Reference to the DescriptorWriter instance.
         */
        DescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo) {
            return write(binding, bufferInfo, 1);
        }
    
        /**
         * @brief Writes multiple buffer descriptors to the specified binding.
         * 
         * @param binding The binding index.
         * @param buffersInfo Pointer to the array of buffer descriptor infos.
         * @param count The number of descriptors.
         * 
         * @return Reference to the DescriptorWriter instance.
         */
        DescriptorWriter& writeBuffers(uint32_t binding, VkDescriptorBufferInfo* buffersInfo, uint32_t count) {
            return write(binding, buffersInfo, count);
        }
    
        /**
         * @brief Writes a single image descriptor to the specified binding.
         * 
         * @param binding The binding index.
         * @param imageInfo Pointer to the image descriptor info.
         * 
         * @return Reference to the DescriptorWriter instance.
         */
        DescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo) {
            return write(binding, imageInfo, 1);
        }
    
        /**
         * @brief Writes multiple image descriptors to the specified binding.
         * 
         * @param binding The binding index.
         * @param imagesInfo Pointer to the array of image descriptor infos.
         * @param count The number of descriptors.
         * 
         * @return Reference to the DescriptorWriter instance.
         */
        DescriptorWriter& writeImages(uint32_t binding, VkDescriptorImageInfo* imagesInfo, uint32_t count) {
            return write(binding, imagesInfo, count);
        }


        /**
         * @brief Overwrites an existing descriptor set with the stored writes.
         * 
         * @param set Reference to the descriptor set to be overwritten.
         */
        void updateSet(VkDescriptorSet& set);

    private:
        /**
         * @brief Generic template function to write descriptor data.
         * 
         * @tparam T Type of descriptor info (VkDescriptorBufferInfo or VkDescriptorImageInfo).
         * @param binding The binding index.
         * @param info Pointer to descriptor info.
         * @param count Number of descriptors.
         * 
         * @return Reference to the DescriptorWriter instance.
         */
        template <typename T>
        DescriptorWriter& write(uint32_t binding, T* info, uint32_t count) {
            PXT_ASSERT(m_setLayout.m_bindings.count(binding) == 1, "Layout does not contain specified binding");
            
            auto& bindingDescription = m_setLayout.m_bindings[binding];
            
            PXT_ASSERT(bindingDescription.descriptorCount == count, "Binding descriptor info count mismatch");
            
            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.descriptorType = bindingDescription.descriptorType;
            write.dstBinding = binding;
            write.descriptorCount = count;
            
            if constexpr (std::is_same_v<T, VkDescriptorBufferInfo>) {
                write.pBufferInfo = info;
            } else if constexpr (std::is_same_v<T, VkDescriptorImageInfo>) {
                write.pImageInfo = info;
            } else {
            	PXT_STATIC_ASSERT(false, "Unsupported type for descriptor write");
            }
            
            m_writes.push_back(write);
            return *this;
        }

		Context& m_context;
        DescriptorSetLayout& m_setLayout;
        std::vector<VkWriteDescriptorSet> m_writes;
    };
}
