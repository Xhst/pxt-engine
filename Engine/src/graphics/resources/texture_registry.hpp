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
		uint32_t add(const Shared<Image>& image) {
			auto* texture = dynamic_cast<Texture2D*>(image.get());

			if (!texture) {
				return 0;
			}

			// Add the resource to the list
			const auto index = static_cast<uint32_t>(m_textures.size());
			m_textures.push_back(image);

			m_idToIndex[image->getId()] = index;

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

	private:
		std::vector<Shared<Image>> m_textures;
		std::unordered_map<ResourceId, uint32_t> m_idToIndex;
	};
}