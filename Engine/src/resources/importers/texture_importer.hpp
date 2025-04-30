#pragma once

#include "core/memory.hpp"
#include "resources/types/image.hpp"
#include "resources/resource_manager.hpp"

#include <filesystem>

namespace PXTEngine {

	class TextureImporter {
	public:
		static Shared<Image> import(const std::filesystem::path& filePath,
			ResourceManager& rm);
	};
}