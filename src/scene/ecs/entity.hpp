#pragma once

#include "scene/scene.hpp"
#include "scene/ecs/component.hpp"

#include <entt/entt.hpp>

namespace CGEngine {

    class Entity {
    public:
        Entity() = default;
        Entity(entt::entity entity, Scene* scene) : m_enttEntity(entity), m_scene(scene) {}

        operator entt::entity() const { return m_enttEntity; }
        operator bool() const { return m_enttEntity != entt::null; }

        /**
         * @brief Check if entity has a component
         * 
         * @tparam T Component type
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
            assert(has<T>() && "Entity does not have component");

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
        template <typename ComponentT>
        void remove() {
            static_assert(!std::is_same<T, IDComponent>::value, "Cannot remove ID component");
            assert(has<Component>() && "Entity does not have component");

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
