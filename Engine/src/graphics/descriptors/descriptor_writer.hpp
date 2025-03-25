#pragma once

#include "graphics/descriptors/descriptor_set_layout.hpp"
#include "graphics/descriptors/descriptor_pool.hpp"

namespace PXTEngine {
    class DescriptorWriter {
    public:
        DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool);

        DescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        DescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        DescriptorSetLayout& m_setLayout;
        DescriptorPool& m_pool;
        std::vector<VkWriteDescriptorSet> m_writes;
    };
}
