#pragma once

#include "core/memory.hpp"
#include "resources/resource.hpp"
#include "resources/importers/resource_importer.hpp"
#include "graphics/context/context.hpp"

#include <unordered_map>
#include <filesystem>
#include <string>
#include <iostream>

namespace PXTEngine {

      class ResourceManager {
      public:
            ResourceManager(Context& context) : m_context(context) {}

            template<typename T>
            Shared<T> get(const ResourceId& id) {
                  return std::static_pointer_cast<T>(get(id));
            }

            Shared<Resource> get(const ResourceId& id) {
                  auto it = m_resources.find(id);

                  if (it != m_resources.end()) {
                        return it->second;
                  }

                  auto filePath = std::filesystem::path(id);

                  try {
                        auto importedResource = ResourceImporter::import(m_context, filePath);

                        add(importedResource);

                        return importedResource;
                  } catch (const std::exception& e) {
                        std::cerr << "Failed to import resource: " << e.what() << std::endl;
                        return nullptr;
				}
            }

            ResourceId add(Shared<Resource> resource) {
				ResourceId id = resource->getId();
				m_resources[id] = resource;
				return id;
            }

            void foreach(const std::function<void(const Shared<Resource>&)>& function) {
                  for (const auto &resource: m_resources | std::views::values) {
                        function(resource);
                  }
            }
          
      private:
            Context& m_context;

            std::unordered_map<ResourceId, Shared<Resource>> m_resources;
      };
}