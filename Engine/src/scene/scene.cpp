#include "scene/scene.hpp"

#include "scene/ecs/component.hpp"
#include "scene/ecs/entity.hpp"
#include "scene/script/script.hpp"

namespace PXTEngine {

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

    Entity Scene::getMainCameraEntity() {
        auto cameraEntities = m_registry.view<CameraComponent, TransformComponent>();
        
        for (auto entity : cameraEntities) {
            if (!cameraEntities.get<CameraComponent>(entity).isMainCamera) continue;
            
            return { entity, this };
        }

        return {};
    }

    void Scene::onStart() {
        getEntitiesWith<ScriptComponent>().each([=](auto entity, auto& scriptComponent) {
            scriptComponent.script = scriptComponent.create();
            scriptComponent.script->m_entity = Entity{ entity, this };
            scriptComponent.script->onCreate();
        });
    }

    void Scene::onUpdate(float delta) {
        getEntitiesWith<ScriptComponent>().each([=](auto entity, auto& scriptComponent) {
            
            scriptComponent.script->onUpdate(delta);
            
        });
    }
}