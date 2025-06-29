#pragma once

#include "core/pch.hpp"
#include "resources/types/image.hpp"
#include "resources/resource_manager.hpp"

namespace PXTEngine {

	class TextureImporter {
	public:
		static Shared<Image> import(ResourceManager& rm, const std::filesystem::path& filePath, 
			ResourceInfo* resourceInfo = nullptr);
	};
}