#pragma once

#include "graphics/context/context.hpp"


namespace PXTEngine {
    /**
     * @brief Manages a Vulkan descriptor set layout.
     *
     * This class encapsulates the creation and lifetime of a `VkDescriptorSetLayout`,
     * using a builder interface to define descriptor bindings.
     */
    class DescriptorSetLayout {
    public:

        /**
         * @brief Builder class for creating a DescriptorSetLayout.
         *
         * This class allows for a more flexible and readable way to create a DescriptorSetLayout
         * by chaining method calls to set various parameters.
         */
        class Builder {
        public:
            explicit Builder(Context& context) : m_context{context} {}

            /**
             * @brief Adds a descriptor binding to the layout.
             *
             * @param binding Binding index (must be unique).
             * @param descriptorType Type of descriptor (e.g., uniform buffer, sampler).
             * @param stageFlags Shader stages that will access the binding.
             * @param count Number of descriptors in the binding (default is 1).
             * @return Reference to the Builder for chaining.
             *
             * @note Throws an assertion failure if the binding is already in use.
             */
            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);

            /**
             * @brief Finalizes and builds the DescriptorSetLayout.
             *
             * @return A unique pointer to the created DescriptorSetLayout.
             * @throws std::runtime_error if Vulkan layout creation fails.
             */
            [[nodiscard]]
            Unique<DescriptorSetLayout> build() const;

        private:
            Context& m_context;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_bindings{};
        };

        DescriptorSetLayout(Context& context, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~DescriptorSetLayout();
        
        DescriptorSetLayout(const DescriptorSetLayout &) = delete;
        DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

        /**
         * @brief Returns the Vulkan descriptor set layout handle.
         * @return VkDescriptorSetLayout object.
         */
        [[nodiscard]]
        VkDescriptorSetLayout getDescriptorSetLayout() const { return m_descriptorSetLayout; }

    private:
        Context& m_context;
        VkDescriptorSetLayout m_descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_bindings;

        friend class DescriptorWriter;
    };
}