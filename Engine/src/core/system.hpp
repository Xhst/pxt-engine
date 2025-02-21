#pragma once

#include "core/uuid.hpp"
#include "core/events/event.hpp"

namespace PXTEngine {
    
    class System {
    public:
        virtual ~System() = default;

        virtual void onInit() {}
        virtual void onUpdate(float deltaTime) {}
        virtual void onEvent(Event& event) {}
        virtual void onShutdown() {}

        UUID getId() const { return m_id; }

    private:
        UUID m_id = UUID();
    };
} 
