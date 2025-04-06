#pragma once

#include "graphics/descriptors/descriptor_pool.hpp"
#include "graphics/descriptors/descriptor_set_layout.hpp"

#include <span>
#include <vector>

namespace PXTEngine {

	struct PoolSizeRatio {
		VkDescriptorType type;
		float ratio;
	};

	class DescriptorAllocatorGrowable {
	public:
		DescriptorAllocatorGrowable(Context& context, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios, 
									float growthFactor = 1.5f, uint32_t maxPools = 4092);

		DescriptorAllocatorGrowable(const DescriptorAllocatorGrowable&) = delete;
		DescriptorAllocatorGrowable& operator=(const DescriptorAllocatorGrowable&) = delete;

		void allocate(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptorSet);

		void resetPools();
		void clearPools();
	private:
		Shared<DescriptorPool> getPool();
		Shared<DescriptorPool> createPool(uint32_t setCount, std::span<PoolSizeRatio> poolRatios);

		Context& m_context;

		std::vector<PoolSizeRatio> m_ratios;
		std::vector<Shared<DescriptorPool>> m_fullPools;
		std::vector<Shared<DescriptorPool>> m_readyPools;

		uint32_t m_setsPerPool;

		float m_growthFactor;
		uint32_t m_maxPools;
	};
}