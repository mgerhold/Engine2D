
#include "Application.hpp"

template<typename DerivedType>
Application<DerivedType>::Application(const std::string& title, WindowSize size, OpenGLVersion version) noexcept
    : mWindow{ title, size, version, mInput },
      mRenderer{ mWindow },
      mAppContext{ .renderer{ mRenderer }, .registry{ mRegistry } } { }

template<typename DerivedType>
Application<DerivedType>::~Application() {
    ScopedTimer::logResults();
}

template<typename DerivedType>
void Application<DerivedType>::run() noexcept {
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
        mTime.delta = currentTime - lastTime;
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

template<typename DerivedType>
void Application<DerivedType>::quit() noexcept {
    glfwSetWindowShouldClose(mWindow.getGLFWWindowPointer(), true);
}