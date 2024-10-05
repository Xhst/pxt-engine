#pragma once

#include "core/uuid.hpp"

#include <entt/entt.hpp>

namespace CGEngine {

    class Entity;

    class Scene {
    public:
        Scene() = default;
		~Scene() = default;
        
        Entity createEntity(const std::string& name = std::string());
        Entity getEntity(UUID uuid);
        
        void destroyEntity(Entity entity);

        template <typename ...T>
        auto getEntitiesWith() {
            return m_registry.view<T...>();
        }

    private:
        std::unordered_map<UUID, entt::entity> m_entityMap; 
        entt::registry m_registry;

        friend class Entity;
    };
}