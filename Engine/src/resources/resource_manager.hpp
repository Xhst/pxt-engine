#pragma once

#include "core/memory.hpp"
#include "resources/resource.hpp"

#include <unordered_map>
#include <functional>
#include <string>

namespace PXTEngine {

	class ResourceManager {
	public:
		ResourceManager() = default;

		
		template<typename T>
        Shared<T> get(const std::string& alias) {
              return std::static_pointer_cast<T>(get(alias));
        }
		
		Shared<Resource> get(const std::string& alias);

		/**
		 * @brief Adds a resource to the manager.
		 * 
		 * @param resource The resource to add.
		 * @param alias The alias to associate with the resource.
		 * 
		 * @return The ID of the added resource.
		 */
		ResourceId add(const Shared<Resource>& resource, const std::string& alias);

		/**
		 * @brief Iterates over all resources and applies the given function to each.
		 * 
		 * @param function The function to apply to each resource.
		 */
		void foreach(const std::function<void(const Shared<Resource>&)>& function);
	          
	private:
	    std::unordered_map<ResourceId, Shared<Resource>> m_resources;
		std::unordered_map<std::string, ResourceId> m_aliases;
	};
}