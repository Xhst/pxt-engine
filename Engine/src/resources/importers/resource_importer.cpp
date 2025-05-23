#include "resources/importers/resource_importer.hpp"

#include "resources/resource.hpp"
#include "resources/importers/texture_importer.hpp"
#include "resources/importers/model_importer.hpp"

#include <unordered_map>
#include <stdexcept>

namespace PXTEngine {

    namespace {
        using ResourceImportFunction = std::function<Shared<Resource>(
            const std::filesystem::path&,
            ResourceManager&
        )>;

        std::unordered_map<std::string, ResourceImportFunction> extensionToImportFunction = {
            {".png", TextureImporter::import},
            {".jpg", TextureImporter::import},
            {".jpeg", TextureImporter::import},
            {".obj", ModelImporter::importObj}
        };
    }

    Shared<Resource> ResourceImporter::import(const std::filesystem::path& filePath,
        ResourceManager& rm) {
        std::string extension = filePath.extension().string();

        auto it = extensionToImportFunction.find(extension);

        if (it == extensionToImportFunction.end()) {
            throw std::runtime_error("Unsupported file extension: " + extension);
        }

        return it->second(filePath, rm);
    }
}