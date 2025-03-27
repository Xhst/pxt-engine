#include "graphics/descriptors/descriptor_writer.hpp"

#include <cassert>
#include <stdexcept>

namespace PXTEngine {    
    DescriptorWriter::DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool)
        : m_setLayout{setLayout}, m_pool{pool} {}

    bool DescriptorWriter::build(VkDescriptorSet& set) {
        bool success = m_pool.allocateDescriptorSet(m_setLayout.getDescriptorSetLayout(), set);
        if (!success) {
            return false;
        }
        overwrite(set);
        return true;
    }

    void DescriptorWriter::overwrite(VkDescriptorSet& set) {
        for (auto& write : m_writes) {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(m_pool.m_device.getDevice(), m_writes.size(), m_writes.data(), 0, nullptr);
    }

}