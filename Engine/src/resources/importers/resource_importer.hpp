#pragma once

#include "core/memory.hpp"
#include "resources/resource.hpp"
#include "graphics/context/context.hpp"

#include <filesystem>

namespace PXTEngine {

    class ResourceImporter {
    public:
        static Shared<Resource> import(Context& context, const std::filesystem::path& filePath);
    };
}