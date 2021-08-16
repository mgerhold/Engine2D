//
// Created by coder2k on 14.08.2021.
//

#pragma once

#include "WindowSize.hpp"
#include "OpenGLVersion.hpp"
#include <spdlog/spdlog.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

class Input;

class Window final {
public:
    Window(const std::string& title, WindowSize size, OpenGLVersion version, Input& input) noexcept;
    Window(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) = delete;
    ~Window();

    [[nodiscard]] inline GLFWwindow* getGLFWWindowPointer() const noexcept {
        return mWindowPtr;
    }
    [[nodiscard]] WindowSize framebufferSize() const;

private:
    static void handleOpenGLDebugOutput(GLenum source,
                                        GLenum type,
                                        unsigned int id,
                                        GLenum severity,
                                        GLsizei /* length */,
                                        const char* message,
                                        const void* /* userParam */);

private:
    GLFWwindow* mWindowPtr;
    WindowSize mFrameBufferSize;
    Input& mInput;
};
