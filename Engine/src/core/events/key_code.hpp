#pragma once

namespace CGEngine {

    enum class KeyCode {
        Unknown,
        
        Backspace,       // Backspace
        Delete,          // Delete
        Tab,             // Tab
        Clear,           // Clear
        Return,          // Return
        Pause,           // Pause
        Escape,          // Escape
        Space,           // Space
        Insert,          // Insert
        Home,            // Home
        End,             // End
        PageUp,          // Page Up
        PageDown,        // Page Down

        // Keypad
        Keypad0,         // Keypad 0
        Keypad1,         // Keypad 1
        Keypad2,         // Keypad 2
        Keypad3,         // Keypad 3
        Keypad4,         // Keypad 4
        Keypad5,         // Keypad 5
        Keypad6,         // Keypad 6
        Keypad7,         // Keypad 7
        Keypad8,         // Keypad 8
        Keypad9,         // Keypad 9
        KeypadPeriod,    // Keypad .
        KeypadDivide,    // Keypad /
        KeypadMultiply,  // Keypad *
        KeypadMinus,     // Keypad -
        KeypadPlus,      // Keypad +
        KeypadEnter,     // Keypad Enter
        KeypadEquals,    // Keypad =

        // Arrow keys
        UpArrow,         // Up Arrow
        DownArrow,       // Down Arrow
        RightArrow,      // Right Arrow
        LeftArrow,       // Left Arrow

        // Function keys
        F1,              // F1
        F2,              // F2
        F3,              // F3
        F4,              // F4
        F5,              // F5
        F6,              // F6
        F7,              // F7
        F8,              // F8
        F9,              // F9
        F10,             // F10
        F11,             // F11
        F12,             // F12
        F13,             // F13
        F14,             // F14
        F15,             // F15

        // Numbers on top of keyboard
        Number0,         // 0
        Number1,         // 1
        Number2,         // 2
        Number3,         // 3
        Number4,         // 4
        Number5,         // 5
        Number6,         // 6
        Number7,         // 7
        Number8,         // 8
        Number9,         // 9

        // Letters
        A,               // A
        B,               // B
        C,               // C
        D,               // D
        E,               // E
        F,               // F
        G,               // G
        H,               // H
        I,               // I
        J,               // J
        K,               // K
        L,               // L
        M,               // M
        N,               // N
        O,               // O
        P,               // P
        Q,               // Q
        R,               // R
        S,               // S
        T,               // T
        U,               // U
        V,               // V
        W,               // W
        X,               // X
        Y,               // Y
        Z,               // Z

        // Special characters
        Exclaim,         // !
        DoubleQuote,     // "
        Hash,            // #
        Dollar,          // $
        Percent,         // %
        Ampersand,       // &
        Quote,           // '
        LeftParen,       // (
        RightParen,      // )
        Asterisk,        // *
        Plus,            // +
        Minus,           // -
        Comma,           // ,
        Period,          // .
        Slash,           // /
        Colon,           // :
        Semicolon,       // ;
        Less,            // <
        Equals,          // =
        Greater,         // >
        Question,        // ?
        At,              // @
        LeftBracket,     // [
        Backslash,       // \
        RightBracket,    // ]
        Caret,           // ^
        Underscore,      // _
        Backquote,       // `

        // Remaining keys
        CapsLock,        // Caps Lock
        ScrollLock,      // Scroll Lock
        NumLock,         // Num Lock
        PrintScreen,     // Print Screen
        PauseBreak,      // Pause/Break
        Menu,            // Menu
        LeftShift,       // Left Shift
        RightShift,      // Right Shift
        LeftControl,     // Left Control
        RightControl,    // Right Control
        LeftAlt,         // Left Alt
        RightAlt,        // Right Alt
        LeftMeta,        // Left Meta (Windows/Command key)
        RightMeta,       // Right Meta (Windows/Command key)
        LeftSuper,       // Left Super (Windows/Command key)
        RightSuper,      // Right Super (Windows/Command key)
        LeftHyper,       // Left Hyper
        RightHyper,      // Right Hyper
        LeftFn,          // Left Fn
        RightFn,         // Right Fn
    };

    enum class MouseButton {
        Unknown,

        Button0,	      // The Left (or primary) mouse button.
        Button1,	      // Right mouse button (or secondary mouse button).
        Button2,	      // Middle mouse button (or third button).
        Button3,	      // Additional (fourth) mouse button.
        Button4,	      // Additional (fifth) mouse button.
        Button5,	      // Additional (or sixth) mouse button.
        Button6,	      // Additional (or seventh) mouse button.
        Button7,	      // Additional (or eighth) mouse button.
        Button8,          // Additional (or ninth) mouse button.
    };

    constexpr MouseButton LeftMouseButton   = MouseButton::Button0;
    constexpr MouseButton RightMouseButton  = MouseButton::Button1;
    constexpr MouseButton MiddleMouseButton = MouseButton::Button2;
}