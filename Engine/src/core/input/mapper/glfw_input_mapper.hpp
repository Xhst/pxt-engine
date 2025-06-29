#pragma once

#include "core/pch.hpp"

#include "core/input/key_code.hpp"

namespace PXTEngine {

    static KeyCode mapGLFWKey(int glfwKey) {
        switch (glfwKey) {
            // Letters
            case GLFW_KEY_A: return KeyCode::A;
            case GLFW_KEY_B: return KeyCode::B;
            case GLFW_KEY_C: return KeyCode::C;
            case GLFW_KEY_D: return KeyCode::D;
            case GLFW_KEY_E: return KeyCode::E;
            case GLFW_KEY_F: return KeyCode::F;
            case GLFW_KEY_G: return KeyCode::G;
            case GLFW_KEY_H: return KeyCode::H;
            case GLFW_KEY_I: return KeyCode::I;
            case GLFW_KEY_J: return KeyCode::J;
            case GLFW_KEY_K: return KeyCode::K;
            case GLFW_KEY_L: return KeyCode::L;
            case GLFW_KEY_M: return KeyCode::M;
            case GLFW_KEY_N: return KeyCode::N;
            case GLFW_KEY_O: return KeyCode::O;
            case GLFW_KEY_P: return KeyCode::P;
            case GLFW_KEY_Q: return KeyCode::Q;
            case GLFW_KEY_R: return KeyCode::R;
            case GLFW_KEY_S: return KeyCode::S;
            case GLFW_KEY_T: return KeyCode::T;
            case GLFW_KEY_U: return KeyCode::U;
            case GLFW_KEY_V: return KeyCode::V;
            case GLFW_KEY_W: return KeyCode::W;
            case GLFW_KEY_X: return KeyCode::X;
            case GLFW_KEY_Y: return KeyCode::Y;
            case GLFW_KEY_Z: return KeyCode::Z;

            // Numbers on the top of the keyboard
            case GLFW_KEY_0: return KeyCode::Number0;
            case GLFW_KEY_1: return KeyCode::Number1;
            case GLFW_KEY_2: return KeyCode::Number2;
            case GLFW_KEY_3: return KeyCode::Number3;
            case GLFW_KEY_4: return KeyCode::Number4;
            case GLFW_KEY_5: return KeyCode::Number5;
            case GLFW_KEY_6: return KeyCode::Number6;
            case GLFW_KEY_7: return KeyCode::Number7;
            case GLFW_KEY_8: return KeyCode::Number8;
            case GLFW_KEY_9: return KeyCode::Number9;

            // Function keys
            case GLFW_KEY_F1:  return KeyCode::F1;
            case GLFW_KEY_F2:  return KeyCode::F2;
            case GLFW_KEY_F3:  return KeyCode::F3;
            case GLFW_KEY_F4:  return KeyCode::F4;
            case GLFW_KEY_F5:  return KeyCode::F5;
            case GLFW_KEY_F6:  return KeyCode::F6;
            case GLFW_KEY_F7:  return KeyCode::F7;
            case GLFW_KEY_F8:  return KeyCode::F8;
            case GLFW_KEY_F9:  return KeyCode::F9;
            case GLFW_KEY_F10: return KeyCode::F10;
            case GLFW_KEY_F11: return KeyCode::F11;
            case GLFW_KEY_F12: return KeyCode::F12;

            // Arrow keys
            case GLFW_KEY_UP:    return KeyCode::UpArrow;
            case GLFW_KEY_DOWN:  return KeyCode::DownArrow;
            case GLFW_KEY_LEFT:  return KeyCode::LeftArrow;
            case GLFW_KEY_RIGHT: return KeyCode::RightArrow;

            // Special keys
            case GLFW_KEY_SPACE: return KeyCode::Space;
            case GLFW_KEY_ESCAPE: return KeyCode::Escape;
            case GLFW_KEY_ENTER: return KeyCode::Return;
            case GLFW_KEY_BACKSPACE: return KeyCode::Backspace;
            case GLFW_KEY_TAB: return KeyCode::Tab;
            case GLFW_KEY_DELETE: return KeyCode::Delete;
            case GLFW_KEY_INSERT: return KeyCode::Insert;
            case GLFW_KEY_HOME: return KeyCode::Home;
            case GLFW_KEY_END: return KeyCode::End;
            case GLFW_KEY_PAGE_UP: return KeyCode::PageUp;
            case GLFW_KEY_PAGE_DOWN: return KeyCode::PageDown;

            // Modifiers
            case GLFW_KEY_LEFT_SHIFT: return KeyCode::LeftShift;
            case GLFW_KEY_RIGHT_SHIFT: return KeyCode::RightShift;
            case GLFW_KEY_LEFT_CONTROL: return KeyCode::LeftControl;
            case GLFW_KEY_RIGHT_CONTROL: return KeyCode::RightControl;
            case GLFW_KEY_LEFT_ALT: return KeyCode::LeftAlt;
            case GLFW_KEY_RIGHT_ALT: return KeyCode::RightAlt;

            // Default case for unmapped keys
            default:
                return KeyCode::Unknown; 
        }
    }

    static int mapToGLFWKey(KeyCode key) {
        switch (key) {
            // Letters
            case KeyCode::A: return GLFW_KEY_A;
            case KeyCode::B: return GLFW_KEY_B;
            case KeyCode::C: return GLFW_KEY_C;
            case KeyCode::D: return GLFW_KEY_D;
            case KeyCode::E: return GLFW_KEY_E;
            case KeyCode::F: return GLFW_KEY_F;
            case KeyCode::G: return GLFW_KEY_G;
            case KeyCode::H: return GLFW_KEY_H;
            case KeyCode::I: return GLFW_KEY_I;
            case KeyCode::J: return GLFW_KEY_J;
            case KeyCode::K: return GLFW_KEY_K;
            case KeyCode::L: return GLFW_KEY_L;
            case KeyCode::M: return GLFW_KEY_M;
            case KeyCode::N: return GLFW_KEY_N;
            case KeyCode::O: return GLFW_KEY_O;
            case KeyCode::P: return GLFW_KEY_P;
            case KeyCode::Q: return GLFW_KEY_Q;
            case KeyCode::R: return GLFW_KEY_R;
            case KeyCode::S: return GLFW_KEY_S;
            case KeyCode::T: return GLFW_KEY_T;
            case KeyCode::U: return GLFW_KEY_U;
            case KeyCode::V: return GLFW_KEY_V;
            case KeyCode::W: return GLFW_KEY_W;
            case KeyCode::X: return GLFW_KEY_X;
            case KeyCode::Y: return GLFW_KEY_Y;
            case KeyCode::Z: return GLFW_KEY_Z;

            // Numbers on the top of the keyboard
            case KeyCode::Number0: return GLFW_KEY_0;
            case KeyCode::Number1: return GLFW_KEY_1;
            case KeyCode::Number2: return GLFW_KEY_2;
            case KeyCode::Number3: return GLFW_KEY_3;
            case KeyCode::Number4: return GLFW_KEY_4;
            case KeyCode::Number5: return GLFW_KEY_5;
            case KeyCode::Number6: return GLFW_KEY_6;
            case KeyCode::Number7: return GLFW_KEY_7;
            case KeyCode::Number8: return GLFW_KEY_8;
            case KeyCode::Number9: return GLFW_KEY_9;

            // Function keys
            case KeyCode::F1:  return GLFW_KEY_F1;
            case KeyCode::F2:  return GLFW_KEY_F2;
            case KeyCode::F3:  return GLFW_KEY_F3;
            case KeyCode::F4:  return GLFW_KEY_F4;
            case KeyCode::F5:  return GLFW_KEY_F5;
            case KeyCode::F6:  return GLFW_KEY_F6;
            case KeyCode::F7:  return GLFW_KEY_F7;
            case KeyCode::F8:  return GLFW_KEY_F8;
            case KeyCode::F9:  return GLFW_KEY_F9;
            case KeyCode::F10: return GLFW_KEY_F10;
            case KeyCode::F11: return GLFW_KEY_F11;
            case KeyCode::F12: return GLFW_KEY_F12;

            // Arrow keys
            case KeyCode::UpArrow:    return GLFW_KEY_UP;
            case KeyCode::DownArrow:  return GLFW_KEY_DOWN;
            case KeyCode::LeftArrow:  return GLFW_KEY_LEFT;
            case KeyCode::RightArrow: return GLFW_KEY_RIGHT;

            // Special keys

            case KeyCode::Space: return GLFW_KEY_SPACE;
            case KeyCode::Escape: return GLFW_KEY_ESCAPE;
            case KeyCode::Return: return GLFW_KEY_ENTER;
            case KeyCode::Backspace: return GLFW_KEY_BACKSPACE;
            case KeyCode::Tab: return GLFW_KEY_TAB;
            case KeyCode::Delete: return GLFW_KEY_DELETE;
            case KeyCode::Insert: return GLFW_KEY_INSERT;
            case KeyCode::Home: return GLFW_KEY_HOME;
            case KeyCode::End: return GLFW_KEY_END;
            case KeyCode::PageUp: return GLFW_KEY_PAGE_UP;
            case KeyCode::PageDown: return GLFW_KEY_PAGE_DOWN;

            // Modifiers
            case KeyCode::LeftShift: return GLFW_KEY_LEFT_SHIFT;
            case KeyCode::RightShift: return GLFW_KEY_RIGHT_SHIFT;
            case KeyCode::LeftControl: return GLFW_KEY_LEFT_CONTROL;
            case KeyCode::RightControl: return GLFW_KEY_RIGHT_CONTROL;
            case KeyCode::LeftAlt: return GLFW_KEY_LEFT_ALT;
            case KeyCode::RightAlt: return GLFW_KEY_RIGHT_ALT;

            // Default case for unmapped keys
            default:
                return GLFW_KEY_UNKNOWN;
        }
    }

    static MouseButton mapGLFWMouseButton(int glfwButton) {
        switch (glfwButton) {
            case GLFW_MOUSE_BUTTON_LEFT: return LeftMouseButton;
            case GLFW_MOUSE_BUTTON_RIGHT: return RightMouseButton;
            case GLFW_MOUSE_BUTTON_MIDDLE: return MiddleMouseButton;
            case GLFW_MOUSE_BUTTON_4: return MouseButton::Button4;
            case GLFW_MOUSE_BUTTON_5: return MouseButton::Button5;
            case GLFW_MOUSE_BUTTON_6: return MouseButton::Button6;
            case GLFW_MOUSE_BUTTON_7: return MouseButton::Button7;
            case GLFW_MOUSE_BUTTON_8: return MouseButton::Button8;

            default: return MouseButton::Unknown;
        }
    }

    static int mapToGLFWMouseButton(MouseButton button) {
        switch (button) {
            case LeftMouseButton: return GLFW_MOUSE_BUTTON_LEFT;
            case RightMouseButton: return GLFW_MOUSE_BUTTON_RIGHT;
            case MiddleMouseButton: return GLFW_MOUSE_BUTTON_MIDDLE;
            case MouseButton::Button4: return GLFW_MOUSE_BUTTON_4;
            case MouseButton::Button5: return GLFW_MOUSE_BUTTON_5;
            case MouseButton::Button6: return GLFW_MOUSE_BUTTON_6;
            case MouseButton::Button7: return GLFW_MOUSE_BUTTON_7;
            case MouseButton::Button8: return GLFW_MOUSE_BUTTON_8;

            default: return GLFW_MOUSE_BUTTON_LAST;
        }
    }
}
