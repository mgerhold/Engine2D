//
// Created by coder2k on 05.07.2021.
//

#include "Input.hpp"
#include <range/v3/all.hpp>
#include <gsl/gsl>
#include <cassert>

Input::Input() noexcept {
    ranges::fill(mKeyBuffer, false);
    mPressedThisFrame.reserve(20);
    mRepeatedThisFrame.reserve(20);
    mReleasedThisFrame.reserve(20);
    ranges::fill(mMouseButtonBuffer, false);
}

bool Input::keyDown(Key key) const noexcept {
    return mKeyBuffer[static_cast<std::size_t>(key)];
}

bool Input::keyPressed(Key key) const noexcept {
    return ranges::contains(mPressedThisFrame, key);
}

bool Input::keyRepeated(Key key) const noexcept {
    return ranges::contains(mRepeatedThisFrame, key);
}

bool Input::keyReleased(Key key) const noexcept {
    return ranges::contains(mReleasedThisFrame, key);
}

bool Input::mouseInsideWindow() const noexcept {
    return mMouseInsideWindow;
}

bool Input::mouseDown(MouseButton button) const noexcept {
    return mMouseButtonBuffer[static_cast<std::size_t>(button)];
}

bool Input::mousePressed(MouseButton button) const noexcept {
    return ranges::contains(mMouseButtonsPressedThisFrame, button);
}

bool Input::mouseReleased(MouseButton button) const noexcept {
    return ranges::contains(mMouseButtonsReleasedThisFrame, button);
}

void Input::keyCallback(int glfwKeyCode, int glfwAction) noexcept {
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
            assert(false && "invalid glfw action");
            break;
    }
}

void Input::mouseCallback(double mouseX, double mouseY) noexcept {
    mMousePosition = glm::vec2{ gsl::narrow_cast<float>(mouseX), gsl::narrow_cast<float>(mouseY) };
}

void Input::mouseButtonCallback(int glfwButton, int glfwAction) noexcept {
    switch (glfwAction) {
        case GLFW_PRESS:
            assert(!mMouseButtonBuffer[glfwButton]);
            mMouseButtonBuffer[glfwButton] = true;
            mMouseButtonsPressedThisFrame.push_back(static_cast<MouseButton>(glfwButton));
            break;
        case GLFW_RELEASE:
            assert(mMouseButtonBuffer[glfwButton]);
            mMouseButtonBuffer[glfwButton] = false;
            mMouseButtonsReleasedThisFrame.push_back(static_cast<MouseButton>(glfwButton));
            break;
        default:
            assert(false && "invalid glfw action");
            break;
    }
}

void Input::nextFrame() noexcept {
    mPressedThisFrame.clear();
    mRepeatedThisFrame.clear();
    mReleasedThisFrame.clear();
    mMouseButtonsPressedThisFrame.clear();
    mMouseButtonsReleasedThisFrame.clear();
}