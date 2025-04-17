#include "graphics/descriptors/descriptor_set_layout.hpp"

#include <cassert>
#include <ranges>
#include <stdexcept>
#include <utility>

namespace PXTEngine {
    DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::addBinding(
        const uint32_t binding,
        const VkDescriptorType descriptorType,
        const VkShaderStageFlags stageFlags,
        const uint32_t count) {

        assert(!m_bindings.contains(binding) && "Binding already in use");

        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;
        m_bindings[binding] = layoutBinding;

        return *this;
    }

    Unique<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const {
        return createUnique<DescriptorSetLayout>(m_context, m_bindings);
    }

    DescriptorSetLayout::DescriptorSetLayout(Context& context,
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings) :
    m_context{context},
    m_bindings{std::move(bindings)} {

        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for (auto val: m_bindings | std::views::values) {
            setLayoutBindings.push_back(val);
        }
        
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();
        
        if (vkCreateDescriptorSetLayout(
                m_context.getDevice(),
                &descriptorSetLayoutInfo,
                nullptr,
                &m_descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    DescriptorSetLayout::~DescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(m_context.getDevice(), m_descriptorSetLayout, nullptr);
    }
}