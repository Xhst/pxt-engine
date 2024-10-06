#pragma once

#include "scene/ecs/entity.hpp"

namespace CGEngine {

    class Script {
    public:
        virtual ~Script() = default;

        virtual void onCreate() {}
        virtual void onUpdate(float deltaTime) {}
        virtual void onDestroy() {}

        template <typename T>
        T& get() {
            return m_entity.get<T>();
        }

    private:
        Entity m_entity;  

        friend class Scene;
    };
}