#pragma once

#include "graphics/context/context.hpp"

#include <vector>
#include <span>

namespace PXTEngine { 
    class DescriptorPool {
    public:
        class Builder {
        public:
            Builder(Context& context) : m_context{context} {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& addPoolSizes(std::span<VkDescriptorPoolSize> poolSizes);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            Unique<DescriptorPool> build() const;

        private:
            Context& m_context;
            std::vector<VkDescriptorPoolSize> m_poolSizes{};
            uint32_t m_maxSets = 1000;
            VkDescriptorPoolCreateFlags m_poolFlags = 0;
        };

        DescriptorPool(Context& context, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags,
                       const std::vector<VkDescriptorPoolSize> &poolSizes);

        ~DescriptorPool();
        
        DescriptorPool(const DescriptorPool &) = delete;
        DescriptorPool &operator=(const DescriptorPool &) = delete;

		DescriptorPool(DescriptorPool&&) = default;

        VkDescriptorPool getDescriptorPool() {return m_descriptorPool;}

        bool allocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor,
								   const void* pNext = nullptr) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

    private:
        Context& m_context;
        VkDescriptorPool m_descriptorPool;

        friend class DescriptorWriter;
    };
}