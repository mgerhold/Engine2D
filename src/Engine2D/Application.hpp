//
// Created by coder2k on 15.05.2021.
//

#pragma once

#include "WindowSize.hpp"
#include "OpenGLVersion.hpp"
#include "ScopedTimer.hpp"

#include <spdlog/spdlog.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Input.hpp"
#include "Registry.hpp"
#include "Component.hpp"

#include <chrono>
#include <vector>
#include <cmath>
#include <string>
#include <iostream>
#include <concepts>
#include <ratio>

template<typename DerivedType>
class Application {
public:
    Application(const std::string& title, WindowSize size, OpenGLVersion version) noexcept;
    Application(const Application&) = delete;
    Application(Application&&) = delete;
    virtual ~Application();
    Application& operator=(const Application&) = delete;
    Application& operator=(Application&&) = delete;

    void run() noexcept;
    [[nodiscard]] bool hasError() const {
        return mError;
    };
    [[nodiscard]] WindowSize getFramebufferSize() const;

protected:
    [[nodiscard]] inline GLFWwindow* getGLFWWindowPointer() const noexcept {
        return mWindowPtr;
    }

protected:
    Input mInput;
    Registry mRegistry;

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
    bool mError{ false };
    WindowSize mFrameBufferSize;
};

#include "Application.inc"
