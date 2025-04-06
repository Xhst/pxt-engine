#include <algorithm>

#include "graphics/descriptors/descriptor_allocator.hpp"

#include <stdexcept>

namespace PXTEngine {

	DescriptorAllocatorGrowable::DescriptorAllocatorGrowable(Context& context, uint32_t maxSets, 
		std::span<PoolSizeRatio> poolRatios, float growthFactor, uint32_t maxPools)
		: m_context{ context }, m_setsPerPool{ maxSets }, m_growthFactor(growthFactor), m_maxPools(maxPools) {

		m_ratios.reserve(poolRatios.size());

		std::vector<VkDescriptorPoolSize> poolSizes;
		poolSizes.reserve(poolRatios.size());

		for (auto& ratio : poolRatios) {
			m_ratios.emplace_back(ratio);

			poolSizes.emplace_back(ratio.type, ratio.ratio * maxSets);
		}

		Shared<DescriptorPool> newPool = createPool(maxSets, poolRatios);

		// Growth for the next allocation
		m_setsPerPool = m_setsPerPool * m_growthFactor;

		m_readyPools.push_back(newPool);
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
		m_setsPerPool = m_setsPerPool * m_growthFactor;

		// Sets per pool is capped to the max pools
		m_setsPerPool = std::min(m_setsPerPool, m_maxPools);


		return newPool;
	}

	Shared<DescriptorPool> DescriptorAllocatorGrowable::createPool(uint32_t setCount, std::span<PoolSizeRatio> poolRatios) {

		std::vector<VkDescriptorPoolSize> poolSizes;
		for (PoolSizeRatio ratio : poolRatios) {
			poolSizes.emplace_back(ratio.type, static_cast<uint32_t>(ratio.ratio * setCount));
		}

		Shared<DescriptorPool> pool = DescriptorPool::Builder(m_context)
			.addPoolSizes(poolSizes)
			.setMaxSets(setCount)
			.build();

		return createShared<DescriptorPool>(m_context, setCount, 0, poolSizes);
	}

	void DescriptorAllocatorGrowable::allocate(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) {
		Shared<DescriptorPool> pool = getPool();

		bool isAllocationSuccessful = pool->allocateDescriptorSet(descriptorSetLayout, descriptor);

		if (!isAllocationSuccessful) {
			// If the allocation failed, we need to move the pool to the full pools list
			m_fullPools.push_back(pool);

			// Try to allocate again from the next pool
			pool = getPool();

			isAllocationSuccessful = pool->allocateDescriptorSet(descriptorSetLayout, descriptor);

			if (!isAllocationSuccessful) {
				throw std::runtime_error("Failed to allocate descriptor set!");
			}
		}

		m_readyPools.push_back(pool);

	}

	void DescriptorAllocatorGrowable::resetPools() {
		for (auto& pool : m_readyPools) {
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