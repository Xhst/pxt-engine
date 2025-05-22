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

	struct MaterialData {
		glm::vec4 albedoColor{ 1.f };
		int albedoMapIndex = 0;
		int normalMapIndex = 1;
		int ambientOcclusionMapIndex = 0;
		int metallicMapIndex = 0;
		int roughnessMapIndex = 0;
	};

	class MaterialRegistry {
	public:
		MaterialRegistry(Context& context, TextureRegistry& textureRegistry) 
			: m_context(context), m_textureRegistry(textureRegistry) {
			m_materialDescriptorSet = VK_NULL_HANDLE;
			m_materialDescriptorSetLayout = nullptr;
			m_descriptorAllocator = nullptr;
		}

		void setDescriptorAllocator(Shared<DescriptorAllocatorGrowable> descriptorAllocator) {
			m_descriptorAllocator = descriptorAllocator;
		}

		uint32_t add(const Shared<Material>& material) {
			const auto index = static_cast<uint32_t>(m_materials.size());

			m_materials.push_back(material);
			m_idToIndex[material->id] = index;

			return index;
		}

		uint32_t getIndex(const ResourceId& id) const {
			auto it = m_idToIndex.find(id);

			if (it != m_idToIndex.end()) {
				return it->second;
			}

			return 0;
		}

		uint32_t getMaterialCount() const {
			return static_cast<uint32_t>(m_materials.size());
		}

		std::vector<Shared<Material>> getMaterials() {
			return m_materials;
		}

		VkDescriptorSet getDescriptorSet() {
			return m_materialDescriptorSet;
		}

		VkDescriptorSetLayout getDescriptorSetLayout() {
			return m_materialDescriptorSetLayout->getDescriptorSetLayout();
		}

		void createDescriptorSet() {
			m_materialDescriptorSetLayout = DescriptorSetLayout::Builder(m_context)
				.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 1)
				.build();

			std::vector<MaterialData> materialsData;

			for (auto& material : m_materials) {
				materialsData.push_back(getMaterialData(material));
			}

			VkDeviceSize bufferSize = sizeof(MaterialData) * materialsData.size();

			Unique<VulkanBuffer> stagingBuffer = createUnique<VulkanBuffer>(
				m_context,
				bufferSize,
				1,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			);
			stagingBuffer->map();

			stagingBuffer->writeToBuffer(
				materialsData.data(), 
				sizeof(MaterialData) * materialsData.size()
			);

			Unique<VulkanBuffer> materialsBuffer = createUnique<VulkanBuffer>(
				m_context,
				bufferSize,
				1,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			);

			m_context.copyBuffer(stagingBuffer->getBuffer(), materialsBuffer->getBuffer(), bufferSize);

			auto bufferInfo = materialsBuffer->descriptorInfo();

			m_descriptorAllocator->allocate(
				m_materialDescriptorSetLayout->getDescriptorSetLayout(), 
				m_materialDescriptorSet
			);

			DescriptorWriter(m_context, *m_materialDescriptorSetLayout)
				.writeBuffer(0, &bufferInfo)
				.updateSet(m_materialDescriptorSet);
		}

		MaterialData getMaterialData(Shared<Material> material) {
			MaterialData materialData;

			materialData.albedoColor = material->getAlbedoColor();
			materialData.albedoMapIndex = m_textureRegistry.getIndex(material->getAlbedoMap()->id);
			materialData.normalMapIndex = m_textureRegistry.getIndex(material->getNormalMap()->id);
			materialData.ambientOcclusionMapIndex = m_textureRegistry.getIndex(material->getAmbientOcclusionMap()->id);
			materialData.metallicMapIndex = m_textureRegistry.getIndex(material->getMetallicMap()->id);
			materialData.roughnessMapIndex = m_textureRegistry.getIndex(material->getRoughnessMap()->id);
			
			return materialData;
		}

	private:
		Context& m_context;
		TextureRegistry& m_textureRegistry;
		Shared<DescriptorAllocatorGrowable> m_descriptorAllocator;

		std::vector<Shared<Material>> m_materials;
		std::unordered_map<ResourceId, uint32_t> m_idToIndex;

		VkDescriptorSet m_materialDescriptorSet = VK_NULL_HANDLE;
		Shared<DescriptorSetLayout> m_materialDescriptorSetLayout = nullptr;
	};
}