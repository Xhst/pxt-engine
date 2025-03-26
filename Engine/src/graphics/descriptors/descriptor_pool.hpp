#pragma once

#include "graphics/device.hpp"

namespace PXTEngine { 
    class DescriptorPool {
    public:
        class Builder {
        public:
            Builder(Device& device) : m_device{device} {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            Unique<DescriptorPool> build() const;

        private:
            Device& m_device;
            std::vector<VkDescriptorPoolSize> m_poolSizes{};
            uint32_t m_maxSets = 1000;
            VkDescriptorPoolCreateFlags m_poolFlags = 0;
        };

        DescriptorPool(
            Device& device,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize> &poolSizes);
        ~DescriptorPool();
        
        DescriptorPool(const DescriptorPool &) = delete;
        DescriptorPool &operator=(const DescriptorPool &) = delete;

        VkDescriptorPool getDescriptorPool() {return m_descriptorPool;}

        bool allocateDescriptorSet(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

    private:
        Device& m_device;
        VkDescriptorPool m_descriptorPool;

        friend class DescriptorWriter;
    };
}