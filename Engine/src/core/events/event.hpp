#pragma once

#include <string>

namespace PXTEngine {

    /**
     * @class Event
     * @brief Base class for all event types.
     * 
     * The Event class is the base class for all event types in the engine. 
     * It provides an interface for getting the event type, name, and string representation.
     */
    class Event {
    public:
        virtual ~Event() = default;

        enum class Type
        {
            None = 0,

            // Window events
            WindowClose, 
            WindowResize, 
            WindowMoved, 
            WindowFocus, 
            WindowLostFocus,

            // Keyboard events
            KeyPress, 
            KeyRelease, 
            KeyDown,

            // Mouse events
            MouseButtonPress, 
            MouseButtonRelease, 
            MouseMove, 
            MouseScroll,
        };

        virtual Event::Type getEventType() const = 0;
        virtual std::string getName() const = 0;
        virtual std::string toString() const { return getName(); }

        bool isHandled() const { return m_handled; }
        void markHandled() { m_handled = true; }

    private:
        bool m_handled = false;

    };

}