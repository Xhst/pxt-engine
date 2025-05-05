#include "resources/resource_manager.hpp"

#include "resources/importers/resource_importer.hpp"

#include <filesystem>
#include <iostream>
#include <ranges>

namespace PXTEngine {

	Shared<Material> ResourceManager::defaultMaterial = nullptr;

	Shared<Resource> ResourceManager::get(const std::string& alias, ResourceInfo* resourceInfo) {

		auto aliasIt = m_aliases.find(alias);

		const ResourceId id = aliasIt != m_aliases.end()
			? aliasIt->second    // Retrieve the ID from the alias map.
			: ResourceId(alias); // Try using the alias as a UUID string.

		if (const auto it = m_resources.find(id); it != m_resources.end()) {
			return it->second;
		}

		const auto filePath = std::filesystem::path(alias);

		try {
			auto importedResource = ResourceImporter::import(*this, filePath, resourceInfo);

			add(importedResource, alias);

			return importedResource;
		} catch (const std::exception& e) {
			std::cerr << "Failed to import resource: " << e.what() << '\n';
			return nullptr;
		}
	}

	ResourceId ResourceManager::add(const Shared<Resource>& resource, const std::string& alias) {
		const ResourceId id = resource->id;
		m_resources[id] = resource;
		m_aliases[alias] = id;
		return id;
	}

	void ResourceManager::foreach(const std::function<void(const Shared<Resource>&)>& function) {
		for (const auto& resource : m_resources | std::views::values) {
			function(resource);
		}
	}
}