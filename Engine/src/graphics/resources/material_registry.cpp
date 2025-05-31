#include "graphics/resources/material_registry.hpp"

namespace PXTEngine {

	MaterialRegistry::MaterialRegistry(Context& context, TextureRegistry& textureRegistry)
		: m_context(context), m_textureRegistry(textureRegistry) {
		m_materialDescriptorSet = VK_NULL_HANDLE;
		m_materialDescriptorSetLayout = nullptr;
		m_descriptorAllocator = nullptr;
	}

	void MaterialRegistry::setDescriptorAllocator(Shared<DescriptorAllocatorGrowable> descriptorAllocator) {
		m_descriptorAllocator = descriptorAllocator;
	}

	uint32_t MaterialRegistry::add(const Shared<Material>& material) {
		const auto index = static_cast<uint32_t>(m_materials.size());
		m_materials.push_back(material);
		m_idToIndex[material->id] = index;
		return index;
	}

	uint32_t MaterialRegistry::getIndex(const ResourceId& id) const {
		auto it = m_idToIndex.find(id);
		return it != m_idToIndex.end() ? it->second : 0;
	}

	VkDescriptorSet MaterialRegistry::getDescriptorSet() {
		return m_materialDescriptorSet;
	}

	VkDescriptorSetLayout MaterialRegistry::getDescriptorSetLayout() {
		return m_materialDescriptorSetLayout->getDescriptorSetLayout();
	}

	void MaterialRegistry::createDescriptorSet() {
		m_materialDescriptorSetLayout = DescriptorSetLayout::Builder(m_context)
			.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 1)
			.build();

		std::vector<MaterialData> materialsData;
		for (const auto& material : m_materials) {
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
		stagingBuffer->writeToBuffer(materialsData.data(), bufferSize);
		stagingBuffer->unmap();

		m_materialsGpuBuffer = createUnique<VulkanBuffer>(
			m_context,
			bufferSize,
			1,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		m_context.copyBuffer(stagingBuffer->getBuffer(), m_materialsGpuBuffer->getBuffer(), bufferSize);

		auto bufferInfo = m_materialsGpuBuffer->descriptorInfo();

		m_descriptorAllocator->allocate(
			m_materialDescriptorSetLayout->getDescriptorSetLayout(),
			m_materialDescriptorSet
		);

		DescriptorWriter(m_context, *m_materialDescriptorSetLayout)
			.writeBuffer(0, &bufferInfo)
			.updateSet(m_materialDescriptorSet);
	}

	MaterialData MaterialRegistry::getMaterialData(Shared<Material> material) {
		MaterialData data;
		data.albedoColor = material->getAlbedoColor();
		data.albedoMapIndex = m_textureRegistry.getIndex(material->getAlbedoMap()->id);
		data.normalMapIndex = m_textureRegistry.getIndex(material->getNormalMap()->id);
		data.ambientOcclusionMapIndex = m_textureRegistry.getIndex(material->getAmbientOcclusionMap()->id);
		data.metallicMapIndex = m_textureRegistry.getIndex(material->getMetallicMap()->id);
		data.roughnessMapIndex = m_textureRegistry.getIndex(material->getRoughnessMap()->id);
		return data;
	}
}
