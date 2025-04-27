#pragma once

#include "core/memory.hpp"
#include "resources/resource.hpp"
#include "resources/importers/resource_importer.hpp"

#include <unordered_map>
#include <filesystem>
#include <string>
#include <iostream>

namespace PXTEngine {

	class ResourceManager {
	public:
		ResourceManager() = default;

		/**
		 * @brief Retrieves a resource by its ID, casting it to the specified type.
		 * 
		 * @param id The ID of the resource to retrieve.
		 * @return A shared pointer to the resource, or nullptr if not found.
		 */
		template<typename T>
        Shared<T> get(const ResourceId& id) {
              return std::static_pointer_cast<T>(get(id));
        }

		/**
		 * @brief Retrieves a resource by its ID.
		 * 
		 * @param id The ID of the resource to retrieve.
		 * @return A shared pointer to the resource, or nullptr if not found.
		 */
		Shared<Resource> get(const ResourceId& id) {
	        auto it = m_resources.find(id);

	        if (it != m_resources.end()) {
	            return it->second;
	        }

	        auto filePath = std::filesystem::path(id);

	        try {
	            auto importedResource = ResourceImporter::import(filePath);

	            add(importedResource);

	            return importedResource;
	        } catch (const std::exception& e) {
                std::cerr << "Failed to import resource: " << e.what() << std::endl;
                return nullptr;
			}
	    }

		/**
		 * @brief Adds a resource to the manager.
		 * 
		 * @param resource The resource to add.
		 * @return The ID of the added resource.
		 */
		ResourceId add(const Shared<Resource>& resource) {
			ResourceId id = resource->getId();
			m_resources[id] = resource;
			return id;
	    }

		/**
		 * @brief Iterates over all resources and applies the given function to each.
		 * 
		 * @param function The function to apply to each resource.
		 */
		void foreach(const std::function<void(const Shared<Resource>&)>& function) {
            for (const auto &resource: m_resources | std::views::values) {
                function(resource);
            }
        }
	          
	private:
	    std::unordered_map<ResourceId, Shared<Resource>> m_resources;
	};
}