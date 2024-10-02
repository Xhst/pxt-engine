#include "scene/scene.hpp"
#include "scene/ecs/entity.hpp"
#include "scene/ecs/component.hpp"

namespace CGEngine {

    Entity Scene::createEntity(const std::string& name) {
        Entity entity = { m_registry.create(), this };

        entity.add<IDComponent>(UUID());
        entity.add<NameComponent>(name.empty() ? "Unnamed-Entity" : name);

        m_entityMap[entity.getUUID()] = entity;

        return entity;
    }

    Entity Scene::getEntity(UUID uuid) {
        assert(m_entityMap.find(uuid) != m_entityMap.end() && "Entity not found in Scene!");

        return { m_entityMap.at(uuid), this };
    }

    void Scene::destroyEntity(Entity entity) {
        m_entityMap.erase(entity.getUUID());
        m_registry.destroy(entity);
    }
}