#include "graphics/descriptors/descriptor_writer.hpp"

namespace PXTEngine {    
    DescriptorWriter::DescriptorWriter(Context& context, DescriptorSetLayout& setLayout)
        : m_context(context), m_setLayout(setLayout) {}

    void DescriptorWriter::updateSet(VkDescriptorSet& set) {
        for (auto& write : m_writes) {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(m_context.getDevice(), m_writes.size(), m_writes.data(), 0, nullptr);
    }

}