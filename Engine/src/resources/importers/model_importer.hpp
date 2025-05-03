#pragma once

#include "core/memory.hpp"
#include "resources/types/mesh.hpp"
#include "resources/resource_manager.hpp"

#include <filesystem>
#include <optional>

namespace PXTEngine {

	class ModelImporter {
	public:
		static Shared<Mesh> importObj(ResourceManager& rm, const std::filesystem::path& filePath,
			ResourceInfo* resourceInfo = nullptr);
	};
}
