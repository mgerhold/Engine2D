//
// Created by coder2k on 05.07.2021.
//

#include "Input.hpp"
#include <range/v3/all.hpp>
#include <algorithm>
#include <cassert>

Input::Input() noexcept {
    std::fill(mKeyBuffer.begin(), mKeyBuffer.end(), false);
    mPressedThisFrame.reserve(20);
    mRepeatedThisFrame.reserve(20);
    mReleasedThisFrame.reserve(20);
}

bool Input::isKeyDown(Key key) const noexcept {
    return mKeyBuffer[static_cast<std::size_t>(key)];
}

bool Input::wasKeyPressed(Key key) const noexcept {
    return ranges::contains(mPressedThisFrame, key);
}

bool Input::wasKeyRepeated(Key key) const noexcept {
    return ranges::contains(mRepeatedThisFrame, key);
}

bool Input::wasKeyReleased(Key key) const noexcept {
    return ranges::contains(mReleasedThisFrame, key);
}

void Input::keyCallback(int glfwKeyCode, int glfwAction, int glfwModifier) noexcept {
    // TODO: handle key modifiers
    if (glfwKeyCode < 0) {
        return;
    }
    switch (glfwAction) {
        case GLFW_PRESS:
            assert(!mKeyBuffer[static_cast<std::size_t>(glfwKeyCode)]);
            mKeyBuffer[static_cast<std::size_t>(glfwKeyCode)] = true;
            mPressedThisFrame.push_back(static_cast<Key>(glfwKeyCode));
            break;
        case GLFW_RELEASE:
            assert(mKeyBuffer[static_cast<std::size_t>(glfwKeyCode)]);
            mKeyBuffer[static_cast<std::size_t>(glfwKeyCode)] = false;
            mReleasedThisFrame.push_back(static_cast<Key>(glfwKeyCode));
            break;
        case GLFW_REPEAT:
            assert(mKeyBuffer[static_cast<std::size_t>(glfwKeyCode)]);
            mRepeatedThisFrame.push_back(static_cast<Key>(glfwKeyCode));
            break;
        default:
            break;
    }
}

void Input::nextFrame() noexcept {
    mPressedThisFrame.clear();
    mRepeatedThisFrame.clear();
    mReleasedThisFrame.clear();
}