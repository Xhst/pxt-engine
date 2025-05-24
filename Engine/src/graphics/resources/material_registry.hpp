#pragma once

#include "core/memory.hpp"
#include "resources/resource.hpp"
#include "resources/types/material.hpp"
#include "graphics/descriptors/descriptors.hpp"
#include "graphics/resources/vk_buffer.hpp"
#include "graphics/resources/texture_registry.hpp"

#include <vector>
#include <unordered_map>

namespace PXTEngine {

	struct alignas(16) MaterialData {
		glm::vec4 albedoColor;
		int albedoMapIndex;
		int normalMapIndex;
		int ambientOcclusionMapIndex;
		int metallicMapIndex;
		int roughnessMapIndex;
	};

	class MaterialRegistry {
	public:
		MaterialRegistry(Context& context, TextureRegistry& textureRegistry);

		void setDescriptorAllocator(Shared<DescriptorAllocatorGrowable> descriptorAllocator);
		uint32_t add(const Shared<Material>& material);
		uint32_t getIndex(const ResourceId& id) const;
		uint32_t getMaterialCount() const;
		std::vector<Shared<Material>> getMaterials();

		VkDescriptorSet getDescriptorSet();
		VkDescriptorSetLayout getDescriptorSetLayout();
		void createDescriptorSet();

	private:
		MaterialData getMaterialData(Shared<Material> material);

		Context& m_context;
		TextureRegistry& m_textureRegistry;
		Shared<DescriptorAllocatorGrowable> m_descriptorAllocator;

		std::vector<Shared<Material>> m_materials;
		std::unordered_map<ResourceId, uint32_t> m_idToIndex;

		Unique<VulkanBuffer> m_materialsGpuBuffer = nullptr;
		VkDescriptorSet m_materialDescriptorSet = VK_NULL_HANDLE;
		Shared<DescriptorSetLayout> m_materialDescriptorSetLayout = nullptr;
	};
}
