#pragma once

#include "graphics/descriptors/descriptor_set_layout.hpp"
#include "graphics/descriptors/descriptor_pool.hpp"

#include <cassert>
#include <stdexcept>

namespace PXTEngine {
    class DescriptorWriter {
    public:
        DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool);

        DescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo) {
            return write(binding, bufferInfo, 1);
        }
    
        DescriptorWriter& writeBuffers(uint32_t binding, VkDescriptorBufferInfo* buffersInfo, uint32_t count) {
            return write(binding, buffersInfo, count);
        }
    
        DescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo) {
            return write(binding, imageInfo, 1);
        }
    
        DescriptorWriter& writeImages(uint32_t binding, VkDescriptorImageInfo* imagesInfo, uint32_t count) {
            return write(binding, imagesInfo, count);
        }

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        template <typename T>
        DescriptorWriter& write(uint32_t binding, T* info, uint32_t count) {
            assert(m_setLayout.m_bindings.count(binding) == 1 && "Layout does not contain specified binding");
            
            auto& bindingDescription = m_setLayout.m_bindings[binding];
            
            assert(bindingDescription.descriptorCount == count && "Binding descriptor info count mismatch");
            
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
                static_assert(false, "Unsupported type for descriptor write");
            }
            
            m_writes.push_back(write);
            return *this;
        }

        DescriptorSetLayout& m_setLayout;
        DescriptorPool& m_pool;
        std::vector<VkWriteDescriptorSet> m_writes;
    };
}
