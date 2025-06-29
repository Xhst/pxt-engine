#include "graphics/descriptors/descriptor_pool.hpp"

namespace PXTEngine {
    DescriptorPool::Builder& DescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, uint32_t count) {
		VkDescriptorPoolSize poolSize{};
		poolSize.type = descriptorType;
		poolSize.descriptorCount = count;

        m_poolSizes.emplace_back(descriptorType, count);
        return *this;
    }

    DescriptorPool::Builder& DescriptorPool::Builder::addPoolSizes(std::span<VkDescriptorPoolSize> poolSizes) {
        for (auto& poolSize : poolSizes) {
            m_poolSizes.emplace_back(poolSize);
        }
        return *this;
    }

    DescriptorPool::Builder& DescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags) {
        m_poolFlags = flags;
        return *this;
    }
    DescriptorPool::Builder& DescriptorPool::Builder::setMaxSets(uint32_t count) {
        m_maxSets = count;
        return *this;
    }

    Unique<DescriptorPool> DescriptorPool::Builder::build() const {
        return createUnique<DescriptorPool>(m_context, m_maxSets, m_poolFlags, m_poolSizes);
    }

    DescriptorPool::DescriptorPool(Context& context, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags,
                                   const std::vector<VkDescriptorPoolSize>& poolSizes) : m_context{context} {
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        descriptorPoolInfo.flags = poolFlags;
        
        if (vkCreateDescriptorPool(m_context.getDevice(), &descriptorPoolInfo, nullptr, &m_descriptorPool) !=
            VK_SUCCESS) { 
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    DescriptorPool::~DescriptorPool() {
        vkDestroyDescriptorPool(m_context.getDevice(), m_descriptorPool, nullptr);
    }

    bool DescriptorPool::allocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, 
                                               VkDescriptorSet& descriptor, const void* pNext) const {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;
		allocInfo.pNext = pNext;

        VkResult result = vkAllocateDescriptorSets(m_context.getDevice(), &allocInfo, &descriptor);

		if (result == VK_ERROR_FRAGMENTED_POOL || result == VK_ERROR_OUT_OF_POOL_MEMORY) {
			return false;
		}

		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("failed to allocate descriptor set, device out of memory!");
		}

		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("failed to allocate descriptor set, host out of memory!");
		}

		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor set!");
		}

        return true;
    }

    void DescriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const {
        vkFreeDescriptorSets(
            m_context.getDevice(),
            m_descriptorPool,
            static_cast<uint32_t>(descriptors.size()),
            descriptors.data());
    }

    void DescriptorPool::resetPool() {
        vkResetDescriptorPool(m_context.getDevice(), m_descriptorPool, 0);
    }

}