//
// Created by coder2k on 15.05.2021.
//

#pragma once

#include "WindowSize.hpp"
#include "OpenGLVersion.hpp"
#include "ScopedTimer.hpp"

#include "Registry.hpp"
#include "Component.hpp"
#include "Window.hpp"
#include "Input.hpp"
#include "AssetDatabase.hpp"
#include "Renderer.hpp"
#include "Time.hpp"
#include "Random.hpp"

namespace c2k {

    template<typename DerivedType>
    class Application {
    public:
        Application(const std::string& title, WindowSize size, OpenGLVersion version) noexcept
            : mWindow{ title, size, version, mInput },
              mRenderer{ mWindow },
              mAppContext{ mRenderer, mRegistry } { }
        Application(const Application&) = delete;
        Application(Application&&) = delete;
        virtual ~Application() {
            ScopedTimer::logResults();
        }
        Application& operator=(const Application&) = delete;
        Application& operator=(Application&&) = delete;
        void run() noexcept {
            using namespace std::chrono_literals;

            static_cast<DerivedType*>(this)->setup();

            const auto frameTimeOutputInterval = 500ms;
            auto lastFrameTimeOutput = std::chrono::high_resolution_clock::now() - frameTimeOutputInterval;
            auto lastTime = std::chrono::high_resolution_clock::now();
            unsigned long numFramesDuringOutputInterval = 0UL;
            while (!glfwWindowShouldClose(mWindow.getGLFWWindowPointer())) {
                static_cast<DerivedType*>(this)->update();
                mRegistry.runSystems();
                glfwSwapBuffers(mWindow.getGLFWWindowPointer());
                mInput.nextFrame();
                glfwPollEvents();
                ++numFramesDuringOutputInterval;
                const auto currentTime = std::chrono::high_resolution_clock::now();
                mTime.delta = std::chrono::duration<double>(currentTime - lastTime).count();
                mTime.elapsed += mTime.delta;
                lastTime = currentTime;
                if (currentTime >= lastFrameTimeOutput + frameTimeOutputInterval) {
                    const double framesPerSecond =
                            static_cast<double>(numFramesDuringOutputInterval) /
                            std::chrono::duration<double, std::ratio<1, 1>>(currentTime - lastFrameTimeOutput).count();
                    lastFrameTimeOutput = currentTime;
                    glfwSetWindowTitle(
                            mWindow.getGLFWWindowPointer(),
                            fmt::format("{:.2f} ms ({:.2f} fps)", 1000.0 / framesPerSecond, framesPerSecond).c_str());
                    numFramesDuringOutputInterval = 0UL;
                }
            }
        }

        void quit() noexcept {
            glfwSetWindowShouldClose(mWindow.getGLFWWindowPointer(), true);
        }


    protected:
        Input mInput;
        Window mWindow;
        Registry mRegistry;
        Renderer mRenderer;
        ApplicationContext mAppContext;
        AssetDatabase mAssetDatabase;
        Time mTime;
        Random mRandom;
    };

}// namespace c2k