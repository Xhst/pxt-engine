#pragma once

#include "graphics/descriptors/descriptor_pool.hpp"
#include "graphics/descriptors/descriptor_set_layout.hpp"

#include <span>
#include <vector>

namespace PXTEngine {
	/**
	 * @brief Represents a ratio for a specific descriptor type used when allocating descriptor pools.
	 *
	 * This structure is used to determine how many descriptors of each type should be created
	 * relative to the total number of descriptor sets in a pool.
	 */
	struct PoolSizeRatio {
		VkDescriptorType type;
		float ratio;
	};

	/**
	 * @brief A dynamic descriptor pool allocator that grows based on demand.
	 *
	 * DescriptorAllocatorGrowable manages Vulkan descriptor pools and allows for dynamic allocation
	 * of descriptor sets. When no existing pool can satisfy an allocation request, it automatically
	 * creates a new pool with increased capacity based on a configurable growth factor.
	 *
	 * The allocator maintains separate lists for ready and full pools, and can reset or clear them as needed.
	 *
	 * @note Pools grow until a user-defined maximum set count is reached.
	 */
	class DescriptorAllocatorGrowable {
	public:
		DescriptorAllocatorGrowable(Context& context, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios, 
									float growthFactor = 1.5f, uint32_t maxPools = 4092);

		DescriptorAllocatorGrowable(const DescriptorAllocatorGrowable&) = delete;
		DescriptorAllocatorGrowable& operator=(const DescriptorAllocatorGrowable&) = delete;

		/**
		 * @brief Allocates a descriptor set from a pool.
		 *
		 * If allocation from the first pool fails, it attempts again with a new pool.
		 *
		 * @param descriptorSetLayout Layout used for the descriptor set.
		 * @param descriptorSet Reference to the descriptor set to be allocated.
		 * @throws std::runtime_error If allocation fails from all pools.
		 */
		void allocate(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptorSet);

		/**
		 * @brief Resets all descriptor pools managed by the allocator.
		 *
		 * Moves all full pools back into the ready pool list after resetting them.
		 */
		void resetPools();

		/**
		 * @brief Clears all descriptor pools.
		 *
		 * Empties both ready and full pool lists.
		 */
		void clearPools();
	private:
		/**
		 * @brief Grows the internal sets-per-pool count by the growth factor, clamped to maxPools.
		 */
		void growSetCount();

		/**
		 * @brief Retrieves an available descriptor pool or creates a new one.
		 *
		 * If no pools are ready, a new pool is created using the current growth parameters.
		 *
		 * @return A shared pointer to a DescriptorPool.
		 */
		Shared<DescriptorPool> getPool();

		/**
		 * @brief Creates a new descriptor pool based on the provided set count and ratios.
		 *
		 * @param setCount Number of descriptor sets for the pool.
		 * @param poolRatios Ratios used to determine pool sizes.
		 * @return A shared pointer to a newly created DescriptorPool.
		 */
		[[nodiscard]]
		Shared<DescriptorPool> createPool(uint32_t setCount, std::span<PoolSizeRatio> poolRatios) const;

		Context& m_context;

		std::vector<PoolSizeRatio> m_ratios;
		std::vector<Shared<DescriptorPool>> m_fullPools;
		std::vector<Shared<DescriptorPool>> m_readyPools;

		uint32_t m_setsPerPool;

		float m_growthFactor;
		uint32_t m_maxPools;
	};
}