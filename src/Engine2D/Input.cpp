//
// Created by coder2k on 05.07.2021.
//

#include "Input.hpp"
#include <algorithm>
#include <cassert>

bool Input::isKeyDown(Key key) const noexcept {
    const KeyState keyState = mKeyBuffer[static_cast<std::size_t>(key)];
    return keyState == KeyState::Down ||
           keyState == KeyState::Repeated ||
           keyState == KeyState::Pressed;
}

bool Input::wasKeyPressed(Key key) const noexcept {
    return mKeyBuffer[static_cast<std::size_t>(key)] == KeyState::Pressed;
}

bool Input::wasKeyRepeated(Key key) const noexcept {
    return mKeyBuffer[static_cast<std::size_t>(key)] == KeyState::Repeated;
}

bool Input::wasKeyReleased(Key key) const noexcept {
    return mKeyBuffer[static_cast<std::size_t>(key)] == KeyState::Released;
}

void Input::keyCallback(int glfwKeyCode, int glfwAction, int glfwModifier) noexcept {
    // TODO: handle key modifiers
    if (glfwKeyCode < 0) {
        return;
    }
    switch (glfwAction) {
        case GLFW_PRESS:
            mKeyBuffer[static_cast<std::size_t>(glfwKeyCode)] = KeyState::Pressed;
            break;
        case GLFW_REPEAT:
            mKeyBuffer[static_cast<std::size_t>(glfwKeyCode)] = KeyState::Repeated;
            break;
        case GLFW_RELEASE:
            mKeyBuffer[static_cast<std::size_t>(glfwKeyCode)] = KeyState::Released;
            break;
        default:
            break;
    }
}

void Input::nextFrame() noexcept {
    for (auto& keyState : mKeyBuffer) {
        switch (keyState) {
            case KeyState::Released:
                keyState = KeyState::Up;
                continue;
            case KeyState::Pressed:
            case KeyState::Repeated:
                keyState = KeyState::Down;
                continue;
            case KeyState::Down:
            case KeyState::Up:
                break;
            default:
                assert(false && "Invalid key state");
                break;
        }
    }
}