#pragma once

#include "core/pch.hpp"
#include "core/input/key_code.hpp"
#include "core/events/event.hpp"

namespace PXTEngine {

    class MouseButtonEvent : public Event {
	public:
        MouseButton getMouseButton() const { return m_button; }
		
	protected:
		MouseButtonEvent(const MouseButton button) : m_button(button) {}

		MouseButton m_button;
	};

    class MouseButtonPressEvent : public MouseButtonEvent {
    public:
        MouseButtonPressEvent(const MouseButton button) : MouseButtonEvent(button) {}

        Event::Type getEventType() const override { return Event::Type::MouseButtonPress; }
        std::string getName() const override { return "MouseButtonPress"; }

        static Event::Type getStaticType() { return Event::Type::MouseButtonPress; }
    };

    class MouseButtonReleaseEvent : public MouseButtonEvent {
    public:
        MouseButtonReleaseEvent(const MouseButton button) : MouseButtonEvent(button) {}

        Event::Type getEventType() const override { return Event::Type::MouseButtonRelease; }
        std::string getName() const override { return "MouseButtonRelease"; }

        static Event::Type getStaticType() { return Event::Type::MouseButtonRelease; }
    };

    class MouseMoveEvent : public Event {
    public:
        MouseMoveEvent(const double x, const double y) : m_x(x), m_y(y) {}

        double getX() const { return m_x; }
        double getY() const { return m_y; }

        Event::Type getEventType() const override { return Event::Type::MouseMove; }
        std::string getName() const override { return "MouseMove"; }

        static Event::Type getStaticType() { return Event::Type::MouseMove; }

    private:
        double m_x, m_y;
    };

    class MouseScrollEvent : public Event {
    public:
        MouseScrollEvent(const double xOffset, const double yOffset) : m_xOffset(xOffset), m_yOffset(yOffset) {}

        double getXOffset() const { return m_xOffset; }
        double getYOffset() const { return m_yOffset; }

        Event::Type getEventType() const override { return Event::Type::MouseScroll; }
        std::string getName() const override { return "MouseScroll"; }

        static Event::Type getStaticType() { return Event::Type::MouseScroll; }

    private:
        double m_xOffset, m_yOffset;
    };


}