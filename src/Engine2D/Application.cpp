//
// Created by coder2k on 25.09.2021.
//

#include "Application.hpp"

namespace {
    // anonymous namespace for functions and data that deal with measuring the frame time
    using namespace std::chrono_literals;

    constexpr auto frameTimeOutputInterval = 500ms;

    struct FrameTimeMeasurements {
        std::chrono::time_point<std::chrono::high_resolution_clock> lastFrameTimeOutput;
        std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
        unsigned long numFramesDuringOutputInterval;
    };

    [[nodiscard]] FrameTimeMeasurements setupTimeMeasurements() noexcept {
        return FrameTimeMeasurements{ .lastFrameTimeOutput{ std::chrono::high_resolution_clock::now() -
                                                            frameTimeOutputInterval },
                                      .lastTime{ std::chrono::high_resolution_clock::now() },
                                      .numFramesDuringOutputInterval{ 0UL } };
    }

    void makeTimeMeasurementsStep(FrameTimeMeasurements& measurements, c2k::Time& time) noexcept {
        ++measurements.numFramesDuringOutputInterval;
        const auto currentTime = std::chrono::high_resolution_clock::now();
        time.delta = std::chrono::duration<double>(currentTime - measurements.lastTime).count();
        time.elapsed += time.delta;
        measurements.lastTime = currentTime;
        if (currentTime >= measurements.lastFrameTimeOutput + frameTimeOutputInterval) {
            time.meanFramesPerSecond =
                    static_cast<double>(measurements.numFramesDuringOutputInterval) /
                    std::chrono::duration<double, std::ratio<1, 1>>(currentTime - measurements.lastFrameTimeOutput)
                            .count();
            measurements.lastFrameTimeOutput = currentTime;
            measurements.numFramesDuringOutputInterval = 0UL;
        }
    }

}// namespace

namespace c2k {

    Application::Application(const std::string& title, c2k::WindowSize size, c2k::OpenGLVersion version) noexcept
        : mWindow{ title, size, version, mInput },
          mRenderer{ mWindow },
          mAppContext{ mRenderer, mRegistry, mTime, mInput, mAssetDatabase, *this, invalidEntity } {
        mAppContext.mainCameraEntity = mRegistry.createEntity(TransformComponent{}, CameraComponent{});
    }

    Application::~Application() noexcept {
        ScopedTimer::logResults();
    }

    void Application::run() noexcept {
        setup();
        auto timeMeasurements = setupTimeMeasurements();
        while (!glfwWindowShouldClose(mWindow.getGLFWWindowPointer())) {
            update();
            runSystems();
            glfwSwapBuffers(mWindow.getGLFWWindowPointer());
            mInput.nextFrame();
            glfwPollEvents();
            makeTimeMeasurementsStep(timeMeasurements, mTime);
            refreshWindowTitle();
        }
    }

    void Application::quit() noexcept {
        glfwSetWindowShouldClose(mWindow.getGLFWWindowPointer(), true);
    }

    void Application::runSystems() noexcept {
        runScripts();
        animateSprites();
        renderDynamicSprites();
    }

    void Application::animateSprites() noexcept {
        for (auto&& [entity, animation, sprite] :
             mRegistry.componentsMutable<SpriteSheetAnimationComponent, DynamicSpriteComponent>()) {
            const double nextFrameChange = animation.lastFrameChange + animation.frameTime;
            if (mTime.elapsed >= nextFrameChange) {
                animation.currentFrame =
                        (animation.currentFrame + 1) % gsl::narrow_cast<int>(animation.spriteSheet->frames.size());
                sprite.textureRect = animation.spriteSheet->frames[animation.currentFrame].rect;
                animation.lastFrameChange = nextFrameChange;
            }
        }
    }

    void Application::runScripts() noexcept {
        for (auto&& [entity, scriptComponent] : mRegistry.components<ScriptComponent>()) {
            scriptComponent.script->invokeUpdate(entity);
        }
        for (const auto& command : mAppContext.bufferedScriptCommands) {
            std::visit([this](auto&& typedCommand) { typedCommand.process(mAppContext); }, command);
        }
        mAppContext.bufferedScriptCommands.clear();
    }

    void Application::renderDynamicSprites() noexcept {
        const auto cameraTransformMatrix =
                mRegistry.component<TransformComponent>(mAppContext.mainCameraEntity).value().matrix();
        mRenderer.clear(true, true);
        mRenderer.beginFrame(cameraTransformMatrix);
        for (auto&& [entity, root, sprite, transform] :
             mRegistry.components<RootComponent, DynamicSpriteComponent, TransformComponent>()) {
            mRenderer.drawQuad(transform.position, transform.rotation, transform.scale, *(sprite.shaderProgram),
                               *(sprite.texture), sprite.textureRect, sprite.color);
        }
        for (auto&& [entity, relationship, sprite, transform] :
             mRegistry.components<RelationshipComponent, DynamicSpriteComponent, TransformComponent>()) {
            auto transformMatrix = transform.matrix();
            Entity current = relationship.parent;
            transformMatrix = mRegistry.component<TransformComponent>(current).value().matrix() * transformMatrix;
            while (mRegistry.hasComponent<RelationshipComponent>(current)) {
                current = mRegistry.component<RelationshipComponent>(current)->parent;
                transformMatrix = mRegistry.component<TransformComponent>(current).value().matrix() * transformMatrix;
            }
            mRenderer.drawQuad(transformMatrix, *sprite.shaderProgram, *sprite.texture, sprite.textureRect,
                               sprite.color);
        }
        mRenderer.endFrame();
    }

    void Application::refreshWindowTitle() noexcept {
        static std::string titleText = "";
        const auto targetTitleText =
                fmt::format("{:.2f} ms ({:.2f} fps)", mTime.meanFrameTime() * 1000.0, mTime.meanFramesPerSecond);
        if (titleText != targetTitleText) {
            glfwSetWindowTitle(mWindow.getGLFWWindowPointer(), targetTitleText.c_str());
            titleText = targetTitleText;
        }
    }

}// namespace c2k