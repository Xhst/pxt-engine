#pragma once

#include "core/memory.hpp"
#include "resources/types/image.hpp"

#include <filesystem>

namespace PXTEngine {

	class TextureImporter {
	public:
		static Shared<Image> import(const std::filesystem::path& filePath);
	};
}