#include "graphics/resources/texture_registry.hpp"

namespace PXTEngine {

	TextureRegistry::TextureRegistry(Context& context)
		: m_context(context) {
		m_textureDescriptorSet = VK_NULL_HANDLE;
		m_textureDescriptorSetLayout = nullptr;
		m_descriptorAllocator = nullptr;
	}

	void TextureRegistry::setDescriptorAllocator(Shared<DescriptorAllocatorGrowable> descriptorAllocator) {
		m_descriptorAllocator = descriptorAllocator;
	}

	uint32_t TextureRegistry::add(const Shared<Image>& image) {
		auto* texture = dynamic_cast<Texture2D*>(image.get());

		if (!texture) {
			return 0;
		}

		const auto index = static_cast<uint32_t>(m_textures.size());
		m_textures.push_back(image);
		m_idToIndex[image->id] = index;

		return index;
	}

	uint32_t TextureRegistry::getIndex(const ResourceId& id) const {
		auto it = m_idToIndex.find(id);
		return it != m_idToIndex.end() ? it->second : 0;
	}

	VkDescriptorSet TextureRegistry::getDescriptorSet() {
		return m_textureDescriptorSet;
	}

	VkDescriptorSetLayout TextureRegistry::getDescriptorSetLayout() {
		return m_textureDescriptorSetLayout->getDescriptorSetLayout();
	}

	void TextureRegistry::createDescriptorSet() {
		m_textureDescriptorSetLayout = DescriptorSetLayout::Builder(m_context)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, static_cast<uint32_t>(m_textures.size()))
			.build();

		std::vector<VkDescriptorImageInfo> imageInfos;
		for (const auto& image : m_textures) {
			const auto texture = std::static_pointer_cast<Texture2D>(image);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = texture->getImageView();
			imageInfo.sampler = texture->getImageSampler();
			imageInfos.push_back(imageInfo);
		}

		m_descriptorAllocator->allocate(m_textureDescriptorSetLayout->getDescriptorSetLayout(), m_textureDescriptorSet);

		DescriptorWriter(m_context, *m_textureDescriptorSetLayout)
			.writeImages(0, imageInfos.data(), static_cast<uint32_t>(imageInfos.size()))
			.updateSet(m_textureDescriptorSet);
	}
}
