#pragma once

#include <string>

namespace CGEngine {

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