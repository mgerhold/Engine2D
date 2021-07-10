//
// Created by coder2k on 05.07.2021.
//

#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <vector>
#include <array>
#include <cstdint>

enum class Key : std::size_t {
    // Unknown = GLFW_KEY_UNKNOWN,
    Space = GLFW_KEY_SPACE,
    Apostrophe = GLFW_KEY_APOSTROPHE,
    Comma = GLFW_KEY_COMMA,
    Minus = GLFW_KEY_MINUS,
    Period = GLFW_KEY_PERIOD,
    Slash = GLFW_KEY_SLASH,
    Num0 = GLFW_KEY_0,
    Num1 = GLFW_KEY_1,
    Num2 = GLFW_KEY_2,
    Num3 = GLFW_KEY_3,
    Num4 = GLFW_KEY_4,
    Num5 = GLFW_KEY_5,
    Num6 = GLFW_KEY_6,
    Num7 = GLFW_KEY_7,
    Num8 = GLFW_KEY_8,
    Num9 = GLFW_KEY_9,
    Semicolon = GLFW_KEY_SEMICOLON,
    Equal = GLFW_KEY_EQUAL,
    A = GLFW_KEY_A,
    B = GLFW_KEY_B,
    C = GLFW_KEY_C,
    D = GLFW_KEY_D,
    E = GLFW_KEY_E,
    F = GLFW_KEY_F,
    G = GLFW_KEY_G,
    H = GLFW_KEY_H,
    I = GLFW_KEY_I,
    J = GLFW_KEY_J,
    K = GLFW_KEY_K,
    L = GLFW_KEY_L,
    M = GLFW_KEY_M,
    N = GLFW_KEY_N,
    O = GLFW_KEY_O,
    P = GLFW_KEY_P,
    Q = GLFW_KEY_Q,
    R = GLFW_KEY_R,
    S = GLFW_KEY_S,
    T = GLFW_KEY_T,
    U = GLFW_KEY_U,
    V = GLFW_KEY_V,
    W = GLFW_KEY_W,
    X = GLFW_KEY_X,
    Y = GLFW_KEY_Y,
    Z = GLFW_KEY_Z,
    LeftBracket = GLFW_KEY_LEFT_BRACKET,
    Backslash = GLFW_KEY_BACKSLASH,
    RightBracket = GLFW_KEY_RIGHT_BRACKET,
    GraveAccent = GLFW_KEY_GRAVE_ACCENT,
    World1 = GLFW_KEY_WORLD_1,
    World2 = GLFW_KEY_WORLD_2,
    Escape = GLFW_KEY_ESCAPE,
    Enter = GLFW_KEY_ENTER,
    Tab = GLFW_KEY_TAB,
    Backspace = GLFW_KEY_BACKSPACE,
    Insert = GLFW_KEY_INSERT,
    Delete = GLFW_KEY_DELETE,
    Right = GLFW_KEY_RIGHT,
    Left = GLFW_KEY_LEFT,
    Down = GLFW_KEY_DOWN,
    Up = GLFW_KEY_UP,
    PageUp = GLFW_KEY_PAGE_UP,
    PageDown = GLFW_KEY_PAGE_DOWN,
    Home = GLFW_KEY_HOME,
    End = GLFW_KEY_END,
    CapsLock = GLFW_KEY_CAPS_LOCK,
    ScrollLock = GLFW_KEY_SCROLL_LOCK,
    NumLock = GLFW_KEY_NUM_LOCK,
    PrintScreen = GLFW_KEY_PRINT_SCREEN,
    Pause = GLFW_KEY_PAUSE,
    F1 = GLFW_KEY_F1,
    F2 = GLFW_KEY_F2,
    F3 = GLFW_KEY_F3,
    F4 = GLFW_KEY_F4,
    F5 = GLFW_KEY_F5,
    F6 = GLFW_KEY_F6,
    F7 = GLFW_KEY_F7,
    F8 = GLFW_KEY_F8,
    F9 = GLFW_KEY_F9,
    F10 = GLFW_KEY_F10,
    F11 = GLFW_KEY_F11,
    F12 = GLFW_KEY_F12,
    F13 = GLFW_KEY_F13,
    F14 = GLFW_KEY_F14,
    F15 = GLFW_KEY_F15,
    F16 = GLFW_KEY_F16,
    F17 = GLFW_KEY_F17,
    F18 = GLFW_KEY_F18,
    F19 = GLFW_KEY_F19,
    F20 = GLFW_KEY_F20,
    F21 = GLFW_KEY_F21,
    F22 = GLFW_KEY_F22,
    F23 = GLFW_KEY_F23,
    F24 = GLFW_KEY_F24,
    F25 = GLFW_KEY_F25,
    NumPad0 = GLFW_KEY_KP_0,
    NumPad1 = GLFW_KEY_KP_1,
    NumPad2 = GLFW_KEY_KP_2,
    NumPad3 = GLFW_KEY_KP_3,
    NumPad4 = GLFW_KEY_KP_4,
    NumPad5 = GLFW_KEY_KP_5,
    NumPad6 = GLFW_KEY_KP_6,
    NumPad7 = GLFW_KEY_KP_7,
    NumPad8 = GLFW_KEY_KP_8,
    NumPad9 = GLFW_KEY_KP_9,
    NumpadDecimal = GLFW_KEY_KP_DECIMAL,
    NumpadDivide = GLFW_KEY_KP_DIVIDE,
    NumpadMultiply = GLFW_KEY_KP_MULTIPLY,
    NumpadSubtract = GLFW_KEY_KP_SUBTRACT,
    NumpadAdd = GLFW_KEY_KP_ADD,
    NumpadEnter = GLFW_KEY_KP_ENTER,
    NumpadEqual = GLFW_KEY_KP_EQUAL,
    LeftShift = GLFW_KEY_LEFT_SHIFT,
    LeftControl = GLFW_KEY_LEFT_CONTROL,
    LeftAlt = GLFW_KEY_LEFT_ALT,
    LeftSuper = GLFW_KEY_LEFT_SUPER,
    RightShift = GLFW_KEY_RIGHT_SHIFT,
    RightControl = GLFW_KEY_RIGHT_CONTROL,
    RightAlt = GLFW_KEY_RIGHT_ALT,
    RightSuper = GLFW_KEY_RIGHT_SUPER,
    Menu = GLFW_KEY_MENU,
    LastKey = GLFW_KEY_MENU,
};
static_assert(static_cast<int>(Key::LastKey) >= 0);

enum class MouseButton : std::size_t {
    Left = 0,
    Right = 1,
    Middle = 2,
    Button0 = 0,
    Button1 = 1,
    Button2 = 2,
    Button3 = 3,
    Button4 = 4,
    Button5 = 5,
    Button6 = 6,
    Button7 = 7,
    Button8 = 8,
    LastButton = Button8,
};

class Input {
public:
    Input() noexcept;

    [[nodiscard]] bool keyDown(Key key) const noexcept;
    [[nodiscard]] bool keyPressed(Key key) const noexcept;
    [[nodiscard]] bool keyRepeated(Key key) const noexcept;
    [[nodiscard]] bool keyReleased(Key key) const noexcept;
    [[nodiscard]] glm::vec2 mousePosition() const noexcept {
        return mMousePosition;
    }
    [[nodiscard]] bool mouseInsideWindow() const noexcept;
    [[nodiscard]] bool mouseDown(MouseButton button) const noexcept;
    [[nodiscard]] bool mousePressed(MouseButton button) const noexcept;
    [[nodiscard]] bool mouseReleased(MouseButton button) const noexcept;

private:
    enum class KeyState : uint8_t { Down, Up, Pressed, Released, Repeated };

private:
    void keyCallback(int glfwKeyCode, int glfwAction) noexcept;
    void mouseCallback(double mouseX, double mouseY) noexcept;
    void mouseEnterOrLeaveCallback(bool entered) noexcept {
        mMouseInsideWindow = entered;
    }
    void mouseButtonCallback(int glfwButton, int glfwAction) noexcept;
    void nextFrame() noexcept;

private:
    std::array<bool, static_cast<std::size_t>(Key::LastKey)> mKeyBuffer;
    std::vector<Key> mPressedThisFrame;
    std::vector<Key> mRepeatedThisFrame;
    std::vector<Key> mReleasedThisFrame;

    std::array<bool, static_cast<std::size_t>(MouseButton::LastButton)> mMouseButtonBuffer;
    std::vector<MouseButton> mMouseButtonsPressedThisFrame;
    std::vector<MouseButton> mMouseButtonsReleasedThisFrame;
    glm::vec2 mMousePosition{ 0.0f };
    bool mMouseInsideWindow{ false };

    template<typename T>
    friend class Application;
};
