#pragma once

#include "core/pch.hpp"
#include "scene/scene.hpp"
#include "scene/ecs/component.hpp"

namespace PXTEngine {

    class Entity {
    public:
        Entity() = default;
        Entity(entt::entity entity, Scene* scene) : m_enttEntity(entity), m_scene(scene) {}

        operator entt::entity() const { return m_enttEntity; }
        operator bool() const { return m_enttEntity != entt::null; }

        /**
         * @brief Check if entity has a component
         * 
         * @tparam Components type
         * @return true if entity has component, false otherwise
         */
        template <typename... Components>
        bool has() {
            return m_scene->m_registry.all_of<Components...>(m_enttEntity);
        }

        /**
         * @brief Get a component from entity
         * 
         * @tparam Component type
         * @return Reference to component
         */
        template <typename Component>
        Component& get() {
            PXT_ASSERT(has<Component>(), "Entity does not have component");

            return m_scene->m_registry.get<Component>(m_enttEntity);
        }

        /**
         * @brief Add a component to entity
         * 
         * Self referential method to allow chaining of add calls
         * 
         * @tparam Component type
         * @return Reference to entity
         */
        template <typename Component, typename... Args>
        Entity& add(Args&&... args) {
            m_scene->m_registry.emplace<Component>(m_enttEntity, std::forward<Args>(args)...);
            return *this;
        }

        /**
         * @brief Add a component to entity and return a reference to it
         * 
         * @tparam Component type
         * @return Reference to component
         */
        template <typename Component, typename... Args>
        Component& addAndGet(Args&&... args) {
            return m_scene->m_registry.emplace<Component>(m_enttEntity, std::forward<Args>(args)...);
        }

        /**
         * @brief Remove a component from entity
         * 
         * @tparam Component type
         */
        template <typename Component>
        void remove() {
            PXT_STATIC_ASSERT((!std::is_same_v<Component, IDComponent>), "Cannot remove ID component");
            PXT_ASSERT(has<Component>(), "Entity does not have component");

            m_scene->m_registry.remove<Component>(m_enttEntity);
        }

        /**
         * @brief Get the UUID of the entity
         * 
         * @return UUID of the entity
         */
        UUID getUUID() {
            return get<IDComponent>().uuid;
        }

    private:
        entt::entity m_enttEntity{entt::null};
        Scene* m_scene = nullptr;
    };

}
