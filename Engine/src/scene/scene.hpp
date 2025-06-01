#pragma once

#include "core/memory.hpp"
#include "core/uuid.hpp"

#include "scene/environment.hpp"

#include <entt/entt.hpp>

namespace PXTEngine {

    class Entity;

    /**
     * @class Scene
     * @brief Manages a collection of entities and their components.
     * 
     * This class serves as a container for entities and provides functionality for entity creation,
     * retrieval, and destruction. It also manages entity updates and scripting behavior.
     */
    class Scene {
    public:
        Scene() = default;
        ~Scene() = default;
        
        /**
         * @brief Creates a new entity in the scene.
         * @param name Optional name for the entity.
         * @return The created entity.
         */
        Entity createEntity(const std::string& name = std::string());
        
        /**
         * @brief Retrieves an entity by its UUID.
         * @param uuid The UUID of the entity.
         * @return The corresponding entity.
         */
        Entity getEntity(UUID uuid);
        
        /**
         * @brief Destroys an entity and removes it from the scene.
         * @param entity The entity to be destroyed.
         */
        void destroyEntity(Entity entity);

        /**
         * @brief Called when the scene starts.
         * 
         * Initializes scripts attached to entities.
         */
        void onStart();
        
        /**
         * @brief Called every frame to update the scene.
         * @param delta Time elapsed since the last update.
         */
        void onUpdate(float delta);

        /**
         * @brief Retrieves all entities that have the specified components.
         * @tparam T Component types to filter entities.
         * @return A view of the entities with the specified components.
         */
        template <typename ...T>
        auto getEntitiesWith() {
            return m_registry.view<T...>();
        }

        /**
         * @brief Gets the entity designated as the main camera.
         * @return The main camera entity or an empty entity if none exist.
         */
        Entity getMainCameraEntity();

        /**
         * @brief Retrieves the environment settings for the scene.
         * @return A shared pointer to the environment settings.
		 */
        Shared<Environment> getEnvironment() const { return m_environment; }

    private:
        std::unordered_map<UUID, entt::entity> m_entityMap;
        
        // The entity registry for managing components.
        entt::registry m_registry;

		Shared<Environment> m_environment = createShared<Environment>();

        friend class Entity;
    };
}
