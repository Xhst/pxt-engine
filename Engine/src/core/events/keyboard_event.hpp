#pragma once

#include "core/input/key_code.hpp"
#include "core/events/event.hpp"

namespace PXTEngine {

    class KeyBoardEvent : public Event {
    public:
        KeyCode getKeyCode() const { return m_keyCode; }

    protected:
        KeyBoardEvent(KeyCode keyCode): m_keyCode(keyCode) {}

        KeyCode m_keyCode;
    };

    class KeyDownEvent : public KeyBoardEvent {
    public:
        KeyDownEvent(KeyCode keyCode): KeyBoardEvent(keyCode) {}

        Event::Type getEventType() const override { return Event::Type::KeyDown; }
        std::string getName() const override { return "KeyDown"; }

        static Event::Type getStaticType() { return Event::Type::KeyDown; }
    };

    class KeyPressEvent : public KeyBoardEvent {
    public:
        KeyPressEvent(KeyCode keyCode): KeyBoardEvent(keyCode) {}
        KeyPressEvent(KeyCode keyCode, int repeatCount): KeyBoardEvent(keyCode), m_repeatCount(repeatCount) {}

        Event::Type getEventType() const override { return Event::Type::KeyPress; }
        std::string getName() const override { return "KeyPress"; }

        static Event::Type getStaticType() { return Event::Type::KeyPress; }

    private:
        int m_repeatCount = 0;
    };

    class KeyReleaseEvent : public KeyBoardEvent {
    public:
        KeyReleaseEvent(KeyCode keyCode): KeyBoardEvent(keyCode) {}

        Event::Type getEventType() const override { return Event::Type::KeyRelease; }
        std::string getName() const override { return "KeyRelease"; }

        static Event::Type getStaticType() { return Event::Type::KeyRelease; }
    };

}