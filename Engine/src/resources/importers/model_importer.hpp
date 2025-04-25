#pragma once

#include "core/memory.hpp"
#include "resources/types/model.hpp"
#include "graphics/context/context.hpp"

#include <filesystem>

namespace PXTEngine {

	class ModelImporter {
	public:
		static Shared<Model> importObj(Context& context, const std::filesystem::path& filePath);
	};
}
