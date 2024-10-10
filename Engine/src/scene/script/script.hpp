#pragma once

#include "scene/ecs/entity.hpp"

namespace CGEngine {

    /**
     * @brief Base class for creating custom scripts to be attached to entities in a scene.
     * 
     * The `Script` class is meant to be inherited by user-defined scripts that manage 
     * the behavior of entities within the game engine. It provides lifecycle methods 
     * such as `onCreate`, `onUpdate`, and `onDestroy`, which can be overridden to implement 
     * custom behavior.
     */
    class Script {
    public:
        virtual ~Script() = default;

        /**
         * @brief Called when the script is first attached to an entity.
         * 
         * This method is intended to be overridden by derived classes to initialize 
         * any data or state when the script is created. By default, it does nothing.
         */
        virtual void onCreate() {}

        /**
         * @brief Called every frame to update the script logic.
         * 
         * This method is intended to be overridden by derived classes to define what 
         * happens during each frame update. The `deltaTime` parameter provides the time 
         * elapsed since the last frame, allowing for frame-rate-independent updates.
         * 
         * @param deltaTime Time in seconds since the last frame update.
         */
        virtual void onUpdate(float deltaTime) {}

        /**
         * @brief Called when the script is removed or the entity is destroyed.
         * 
         * This method is intended to be overridden by derived classes to clean up 
         * resources or perform actions when the script is destroyed. By default, it does nothing.
         */
        virtual void onDestroy() {}

        /**
         * @brief Retrieves a component attached to the entity that owns this script.
         * 
         * This method allows the script to access components attached to the entity 
         * that it is associated with.
         * 
         * @tparam T The type of the component to retrieve.
         * @return A reference to the component of type T.
         */
        template <typename T>
        T& get() {
            return m_entity.get<T>();
        }

    private:
        Entity m_entity; 

        friend class Scene;
    };
}
