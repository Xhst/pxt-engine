#pragma once

#include "core/events/event.hpp"
#include <functional>

namespace PXTEngine {

    /**
     * @class EventDispatcher
     * @brief A utility class for dispatching events to appropriate handlers.
     *
     * The EventDispatcher class is used to dispatch events to their corresponding
     * event handlers based on the event type. It holds a reference to an Event object
     * and provides a method to dispatch the event if it matches a specific type.
     */
    class EventDispatcher {
    public:
        /**
         * @brief Constructs an EventDispatcher with the given event.
         * @param event The event to be dispatched.
         */
        explicit EventDispatcher(Event& event)
            : m_event(event) {}

        /**
         * @brief Dispatches the event if it matches the specified type.
         *
         * This method checks if the event type matches the specified type T.
         * If it does, it calls the provided function with the event and marks
         * the event as handled.
         *
         * @tparam T The type of the event to dispatch.
         * @tparam F The type of the function to call if the event matches.
         * @param func The function to call if the event matches the specified type.
         * @return True if the event was dispatched and handled, false otherwise.
         */
        template<typename T, typename F>
        bool dispatch(const F& func) {
            // Ensure the event type matches and hasn't been handled yet
            if (m_event.getEventType() == T::getStaticType() && !m_event.isHandled()) {
                // Cast the event to the correct type and invoke the handler
                m_event.markHandled();
                func(static_cast<T&>(m_event));
                return true;
            }
            return false;
        }

    private:
        Event& m_event; 
    };

}
