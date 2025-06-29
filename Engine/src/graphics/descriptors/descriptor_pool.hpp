#pragma once

#include "core/pch.hpp"
#include "graphics/context/context.hpp"

namespace PXTEngine {

    /**
     * @brief Manages a Vulkan descriptor pool and handles descriptor set allocations.
     *
     * This class provides RAII-style management for `VkDescriptorPool` objects, including
     * creation, destruction, and descriptor set allocation. Descriptor sets can be allocated,
     * freed, or the entire pool can be reset.
     */
    class DescriptorPool {
    public:
        /**
         * @brief Builder class for creating a DescriptorPool.
         *
         * This class allows for a more flexible and readable way to create a DescriptorPool
         * by chaining method calls to set various parameters.
         */
        class Builder {
        public:
            explicit Builder(Context& context) : m_context{context} {}

            /**
             * @brief Adds a single descriptor type and count to the pool configuration.
             * @param descriptorType The type of descriptor (e.g., VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER).
             * @param count Number of descriptors of the specified type.
             * @return Reference to the Builder for method chaining.
             */
            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);

            /**
             * @brief Adds multiple pool sizes in one call.
             * @param poolSizes Span of VkDescriptorPoolSize entries.
             * @return Reference to the Builder for method chaining.
             */
            Builder& addPoolSizes(std::span<VkDescriptorPoolSize> poolSizes);

            /**
             * @brief Sets creation flags for the descriptor pool.
             * @param flags Vulkan descriptor pool creation flags.
             * @return Reference to the Builder for method chaining.
             */
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);

            /**
             * @brief Sets the maximum number of descriptor sets the pool can allocate.
             * @param count The max number of descriptor sets.
             * @return Reference to the Builder for method chaining.
             */
            Builder& setMaxSets(uint32_t count);

            /**
             * @brief Builds and returns a unique DescriptorPool instance.
             * @return A unique pointer to the created DescriptorPool.
             */
            [[nodiscard]]
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

        /**
        * @brief Retrieves the underlying Vulkan descriptor pool handle.
        * @return The VkDescriptorPool handle.
        */
        [[nodiscard]]
        VkDescriptorPool getDescriptorPool() const {return m_descriptorPool;}

        /**
         * @brief Allocates a descriptor set from the pool.
         *
         * @param descriptorSetLayout Layout used for the descriptor set.
         * @param descriptor Output reference to the allocated descriptor set.
         * @param pNext Optional pointer for extended allocation info (default: nullptr).
         * @return true if allocation succeeded; false if the pool is out of memory or fragmented.
         * @throws std::runtime_error on critical Vulkan allocation errors.
         */
        bool allocateDescriptorSet(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor,
								   const void* pNext = nullptr) const;

        /**
         * @brief Frees a batch of descriptor sets previously allocated from this pool.
         * @param descriptors The descriptor sets to be freed.
         */
        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        /**
         * @brief Resets the descriptor pool, freeing all descriptor sets.
         *
         * This allows reusing the pool without recreating it.
         */
        void resetPool();

    private:
        Context& m_context;
        VkDescriptorPool m_descriptorPool;

        friend class DescriptorWriter;
    };
}