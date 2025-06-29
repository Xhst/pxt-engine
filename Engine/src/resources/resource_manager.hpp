#pragma once

#include "core/pch.hpp"
#include "resources/resource.hpp"

namespace PXTEngine {

	// forward declaration
	class Material;

	/**
	 * @class ResourceManager
	 *
	 * @brief Manages resources in the engine, allowing for retrieval and storage of resources.
	 */
	class ResourceManager {
	public:
		ResourceManager() = default;
		~ResourceManager();

		/**
		 * @brief Retrieves a resource by its alias and casts it to the specified type.
		 * If the alias is not found, it tries to load the resource using the provided string
		 * as resourceId. If the resource is not found, it returns a nullptr.
		 *
		 * @tparam T The type of the resource to retrieve.
		 * @param alias The alias of the resource to retrieve.
		 * @param resourceInfo Optional pointer to store additional resource information.
		 *
		 * @return A shared pointer to the requested resource of type T.
		 */
		template<typename T>
        Shared<T> get(const std::string& alias, ResourceInfo* resourceInfo = nullptr) {
              return std::static_pointer_cast<T>(get(alias, resourceInfo));
        }

		/**
		 * @brief Retrieves a resource by its alias.
		 * If the alias is not found, it tries to load the resource using the provided string
		 * as resourceId. If the resource is not found, it returns a nullptr.
		 *
		 * @param alias The alias of the resource to retrieve.
		 * @param resourceInfo Optional pointer to store additional resource information.
		 *
		 * @return A shared pointer to the requested resource.
		 */
		Shared<Resource> get(const std::string& alias, ResourceInfo* resourceInfo = nullptr);

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

		static Shared<Material> defaultMaterial;
	          
	private:
	    std::unordered_map<ResourceId, Shared<Resource>> m_resources;
		std::unordered_map<std::string, ResourceId> m_aliases;
	};
}