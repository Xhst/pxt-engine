#include "resources/importers/resource_importer.hpp"

#include "resources/resource.hpp"
#include "resources/importers/texture_importer.hpp"
#include "resources/importers/mesh_importer.hpp"

namespace PXTEngine {

    namespace {
        using ResourceImportFunction = std::function<Shared<Resource>(
            ResourceManager&,
            const std::filesystem::path,
            ResourceInfo* resourceInfo
        )>;

        std::unordered_map<std::string, ResourceImportFunction> extensionToImportFunction = {
            {".png", TextureImporter::import},
            {".jpg", TextureImporter::import},
            {".jpeg", TextureImporter::import},
            {".obj", MeshImporter::importObj}
        };
    }

    Shared<Resource> ResourceImporter::import(ResourceManager& rm, const std::filesystem::path& filePath,
        ResourceInfo* resourceInfo) {

        std::string extension = filePath.extension().string();

        auto it = extensionToImportFunction.find(extension);

        if (it == extensionToImportFunction.end()) {
            throw std::runtime_error("Unsupported file extension: " + extension);
        }

        return it->second(rm, filePath, resourceInfo);
    }
}