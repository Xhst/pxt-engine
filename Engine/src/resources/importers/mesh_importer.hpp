#pragma once

#include "core/pch.hpp"
#include "resources/types/mesh.hpp"
#include "resources/resource_manager.hpp"

namespace PXTEngine {

	class MeshImporter {
	public:
		static Shared<Mesh> importObj(ResourceManager& rm, const std::filesystem::path& filePath,
			ResourceInfo* resourceInfo = nullptr);
	};
}
