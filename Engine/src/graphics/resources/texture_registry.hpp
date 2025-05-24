#pragma once

#include "core/memory.hpp"
#include "resources/resource.hpp"
#include "resources/types/image.hpp"
#include "graphics/descriptors/descriptors.hpp"
#include "graphics/resources/texture2d.hpp"

#include <vector>
#include <unordered_map>

namespace PXTEngine {

	class TextureRegistry {
	public:
		explicit TextureRegistry(Context& context);

		void setDescriptorAllocator(Shared<DescriptorAllocatorGrowable> descriptorAllocator);
		uint32_t add(const Shared<Image>& image);

		[[nodiscard]] uint32_t getIndex(const ResourceId& id) const;
		[[nodiscard]] uint32_t getTextureCount() const;
		std::vector<Shared<Image>> getTextures();

		VkDescriptorSet getDescriptorSet();
		VkDescriptorSetLayout getDescriptorSetLayout();

		void createDescriptorSet();

	private:
		std::vector<Shared<Image>> m_textures;
		std::unordered_map<ResourceId, uint32_t> m_idToIndex;

		Context& m_context;
		Shared<DescriptorAllocatorGrowable> m_descriptorAllocator;
		Shared<DescriptorSetLayout> m_textureDescriptorSetLayout;
		VkDescriptorSet m_textureDescriptorSet;
	};
}
