#include "graphics/descriptors/descriptor_set_layout.hpp"

#include <cassert>
#include <stdexcept>

namespace PXTEngine {
    DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count) {
        assert(m_bindings.count(binding) == 0 && "Binding already in use");
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;
        m_bindings[binding] = layoutBinding;
        return *this;
    }

    Unique<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const {
        return createUnique<DescriptorSetLayout>(m_device, m_bindings);
    }

    DescriptorSetLayout::DescriptorSetLayout(
        Device& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
        : m_device{device}, m_bindings{bindings} {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for (auto kv : m_bindings) {
            setLayoutBindings.push_back(kv.second);
        }
        
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();
        
        if (vkCreateDescriptorSetLayout(
                m_device.getDevice(),
                &descriptorSetLayoutInfo,
                nullptr,
                &m_descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    DescriptorSetLayout::~DescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(m_device.getDevice(), m_descriptorSetLayout, nullptr);
    }
}