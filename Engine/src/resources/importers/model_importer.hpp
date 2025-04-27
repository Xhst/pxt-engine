#pragma once

#include "core/memory.hpp"
#include "resources/types/model.hpp"

#include <filesystem>

namespace PXTEngine {

	class ModelImporter {
	public:
		static Shared<Model> importObj(const std::filesystem::path& filePath);
	};
}
