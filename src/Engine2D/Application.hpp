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

#include "Registry.hpp"
#include "Component.hpp"
#include "Window.hpp"
#include "Input.hpp"
#include "AssetDatabase.hpp"
#include "Renderer.hpp"
#include "Time.hpp"

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

protected:
    Input mInput;
    Window mWindow;
    AssetDatabase mAssetDatabase;
    Registry mRegistry;
    Renderer mRenderer;
    Time mTime;
};

#include "Application.inc"
