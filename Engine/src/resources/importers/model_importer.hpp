#pragma once

#include "core/memory.hpp"
#include "resources/types/mesh.hpp"
#include "resources/resource_manager.hpp"

#include <filesystem>

namespace PXTEngine {

	class ModelImporter {
	public:
		static Shared<Mesh> importObj(const std::filesystem::path& filePath,
			ResourceManager& rm);
	};
}
