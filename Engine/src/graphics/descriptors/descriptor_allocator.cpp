#include "graphics/descriptors/descriptor_allocator.hpp"

namespace PXTEngine {

	DescriptorAllocatorGrowable::DescriptorAllocatorGrowable(Context& context, const uint32_t maxSets,
		std::span<PoolSizeRatio> poolRatios, const float growthFactor, const uint32_t maxPools) :
		m_context(context),
		m_setsPerPool(maxSets),
		m_growthFactor(growthFactor),
		m_maxPools(maxPools) {

		m_ratios.reserve(poolRatios.size());

		std::vector<VkDescriptorPoolSize> poolSizes;
		poolSizes.reserve(poolRatios.size());

		for (auto& ratio : poolRatios) {
			m_ratios.emplace_back(ratio);

			poolSizes.emplace_back(ratio.type, ratio.ratio * maxSets);
		}

		Shared<DescriptorPool> newPool = createPool(maxSets, poolRatios);

		// Growth for the next allocation
		growSetCount();

		m_readyPools.push_back(newPool);
	}

	void DescriptorAllocatorGrowable::growSetCount() {
		m_setsPerPool = static_cast<uint32_t>(m_setsPerPool * m_growthFactor);

		// Sets per pool is capped to the max pools
		m_setsPerPool = std::min(m_setsPerPool, m_maxPools);
	}

	Shared<DescriptorPool> DescriptorAllocatorGrowable::getPool() {
		if (!m_readyPools.empty()) {
			// Get the last ready pool
			Shared<DescriptorPool> pool = m_readyPools.back();

			// Remove it from the ready pool list
			m_readyPools.pop_back();

			return pool;
		}

		// No ready pools, so we need to create a new one
		Shared<DescriptorPool> newPool = createPool(m_setsPerPool, m_ratios);

		// Growth for the next allocation
		growSetCount();

		return newPool;
	}

	Shared<DescriptorPool> DescriptorAllocatorGrowable::createPool(uint32_t setCount, std::span<PoolSizeRatio> poolRatios) const {

		std::vector<VkDescriptorPoolSize> poolSizes;
		for (auto [type, ratio] : poolRatios) {
			poolSizes.emplace_back(type, static_cast<uint32_t>(ratio * setCount));
		}

		Shared<DescriptorPool> pool = DescriptorPool::Builder(m_context)
			.addPoolSizes(poolSizes)
			.setMaxSets(setCount)
			.build();

		return pool;
	}

	void DescriptorAllocatorGrowable::allocate(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptorSet) {
		Shared<DescriptorPool> pool = getPool();

		bool isAllocationSuccessful = pool->allocateDescriptorSet(descriptorSetLayout, descriptorSet);

		if (!isAllocationSuccessful) {
			// If the allocation failed, we need to move the pool to the full pools list
			m_fullPools.push_back(pool);

			// Try to allocate again from the next pool
			pool = getPool();

			isAllocationSuccessful = pool->allocateDescriptorSet(descriptorSetLayout, descriptorSet);

			if (!isAllocationSuccessful) {
				throw std::runtime_error("Failed to allocate descriptor set!");
			}
		}

		m_readyPools.push_back(pool);

	}

	void DescriptorAllocatorGrowable::resetPools() {
		for (const auto& pool : m_readyPools) {
			pool->resetPool();
		}
		
		for (auto& pool : m_fullPools) {
			pool->resetPool();
			m_readyPools.push_back(pool);
		}
		m_fullPools.clear();
	}

	void DescriptorAllocatorGrowable::clearPools() {
		m_readyPools.clear();
		m_fullPools.clear();
	}
}