#pragma once

#include "core/memory.hpp"
#include "resources/resource.hpp"
#include "resources/resource_manager.hpp"

#include <filesystem>
#include <optional>

namespace PXTEngine {

    class ResourceImporter {
    public:
        static Shared<Resource> import(ResourceManager& rm, const std::filesystem::path& filePath,
            ResourceInfo* resourceInfo = nullptr);
    };
}