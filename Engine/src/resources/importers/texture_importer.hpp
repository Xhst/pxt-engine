#pragma once

#include "core/memory.hpp"
#include "resources/types/image.hpp"
#include "graphics/context/context.hpp"

#include <filesystem>

namespace PXTEngine {

	class TextureImporter {
	public:
		static Shared<Image> import(Context& context, const std::filesystem::path& filePath);
	};
}