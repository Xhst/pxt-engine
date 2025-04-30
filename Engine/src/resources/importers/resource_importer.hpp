#pragma once

#include "core/memory.hpp"
#include "resources/resource.hpp"
#include "resources/resource_manager.hpp"

#include <filesystem>

namespace PXTEngine {

    class ResourceImporter {
    public:
        static Shared<Resource> import(const std::filesystem::path& filePath,
            ResourceManager& rm);
    };
}