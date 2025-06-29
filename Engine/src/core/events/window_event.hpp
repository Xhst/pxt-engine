#pragma once

#include "core/pch.hpp"
#include "core/events/event.hpp"

namespace PXTEngine {
     
    class WindowCloseEvent : public Event {
    public:
        WindowCloseEvent() = default;

        Event::Type getEventType() const override { return Event::Type::WindowClose; }
        std::string getName() const override { return "WindowClose"; }

        static Event::Type getStaticType() { return Event::Type::WindowClose; }
    };

    class WindowResizeEvent : public Event {
    public:
        WindowResizeEvent(uint32_t width, uint32_t height)
            : m_width(width), m_height(height) {}

        uint32_t getWidth() const { return m_width; }
        uint32_t getHeight() const { return m_height; }

        Event::Type getEventType() const override { return Event::Type::WindowResize; }
        std::string getName() const override { return "WindowResize"; }

        static Event::Type getStaticType() { return Event::Type::WindowResize; }

    private:
        uint32_t m_width, m_height;
    };
}