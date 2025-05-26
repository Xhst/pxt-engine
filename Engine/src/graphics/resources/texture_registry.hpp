#pragma once

#include "core/memory.hpp"
#include "resources/resource.hpp"
#include "resources/types/image.hpp"
#include "graphics/descriptors/descriptors.hpp"
#include "graphics/resources/texture2d.hpp"

#include <vector>
#include <unordered_map>

namespace PXTEngine {

	/**
	 * @class TextureRegistry
	 *
	 * @brief Manages a collection of textures and their binding to GPU descriptor sets.
	 */
	class TextureRegistry {
	public:
		explicit TextureRegistry(Context& context);

		/**
		 * @brief Sets the descriptor allocator for the registry.
		 *
		 * @param descriptorAllocator Shared pointer to a growable descriptor allocator.
		 */
		void setDescriptorAllocator(Shared<DescriptorAllocatorGrowable> descriptorAllocator);

		/**
		 * @brief Adds a texture to the registry.
		 *
		 * Only 2D textures (Texture2D) are supported. 
		 * If the provided image is not a Texture2D, the function returns 0.
		 *
		 * @param image Shared pointer to the image.
		 *
		 * @return Index of the added texture if valid, otherwise 0.
		 */
		uint32_t add(const Shared<Image>& image);

		/**
		 * @brief Gets the index of a texture in the registry by its resource ID.
		 *
		 * @param id Resource ID of the texture.
		 *
		 * @return Index if found, otherwise 0.
		 */
		[[nodiscard]] uint32_t getIndex(const ResourceId& id) const;

		/**
		 * @brief Returns the Vulkan descriptor set that holds all texture bindings.
		 *
		 * @return Vulkan descriptor set.
		 */
		VkDescriptorSet getDescriptorSet();

		/**
		 * @brief Returns the Vulkan descriptor set layout used for texture bindings.
		 *
		 * @return Vulkan descriptor set layout.
		 */
		VkDescriptorSetLayout getDescriptorSetLayout();

		/**
		 * @brief Creates a Vulkan descriptor set for all registered textures.
		 *
		 * This function constructs a descriptor set layout with a combined image sampler binding,
		 * prepares image info for each texture, allocates the descriptor set, and writes all image bindings to it.
		 */
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
