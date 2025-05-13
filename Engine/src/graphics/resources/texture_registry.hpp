#pragma once

#include "core/memory.hpp"
#include "resources/resource.hpp"
#include "resources/types/image.hpp"
#include "graphics/resources/texture2d.hpp"

#include <vector>
#include <unordered_map>

namespace PXTEngine {

	class TextureRegistry {
	public:
		TextureRegistry(Context& context) : m_context(context) {
			m_textureDescriptorSet = VK_NULL_HANDLE;
			m_textureDescriptorSetLayout = nullptr;
			m_descriptorAllocator = nullptr;
		}

		void setDescriptorAllocator(Shared<DescriptorAllocatorGrowable> descriptorAllocator) {
			m_descriptorAllocator = descriptorAllocator;
		}

		uint32_t add(const Shared<Image>& image) {
			auto* texture = dynamic_cast<Texture2D*>(image.get());

			if (!texture) {
				return 0;
			}

			// Add the resource to the list
			const auto index = static_cast<uint32_t>(m_textures.size());
			m_textures.push_back(image);

			m_idToIndex[image->id] = index;

			return index;
		}

		[[nodiscard]] uint32_t getIndex(const ResourceId& id) const {
			auto it = m_idToIndex.find(id);

			if (it != m_idToIndex.end()) {
				return it->second;
			}

			return 0;
		}

		[[nodiscard]] uint32_t getTextureCount() const {
			return static_cast<uint32_t>(m_textures.size());
		}

		std::vector<Shared<Image>> getTextures() {
			return m_textures;
		}

		VkDescriptorSet getDescriptorSet() {
			return m_textureDescriptorSet;
		}

		VkDescriptorSetLayout getDescriptorSetLayout() {
			return m_textureDescriptorSetLayout->getDescriptorSetLayout();
		}

		void createDescriptorSet() {
			// TEXTURE DESCRIPTOR SET
			m_textureDescriptorSetLayout = DescriptorSetLayout::Builder(m_context)
				.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, getTextureCount())
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

	private:
		std::vector<Shared<Image>> m_textures;
		std::unordered_map<ResourceId, uint32_t> m_idToIndex;

		Context& m_context;

		Shared<DescriptorAllocatorGrowable> m_descriptorAllocator;
		Shared<DescriptorSetLayout> m_textureDescriptorSetLayout;
		VkDescriptorSet m_textureDescriptorSet;
	};
}