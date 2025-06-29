#pragma once

#include "core/pch.hpp"
#include "resources/resource.hpp"
#include "resources/resource_manager.hpp"

namespace PXTEngine {

    class ResourceImporter {
    public:
        static Shared<Resource> import(ResourceManager& rm, const std::filesystem::path& filePath,
            ResourceInfo* resourceInfo = nullptr);
    };
}