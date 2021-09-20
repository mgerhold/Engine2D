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

        void makeTimeMeasurementsStep(FrameTimeMeasurements& measurements, Time& time) noexcept {
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

    template<typename DerivedType>
    class Application {
    public:
        Application(const std::string& title, WindowSize size, OpenGLVersion version) noexcept
            : mWindow{ title, size, version, mInput },
              mRenderer{ mWindow },
              mAppContext{ mRenderer, mRegistry, mTime, mInput, mAssetDatabase, invalidEntity } {
            mAppContext.mainCameraEntity = mRegistry.createEntity(TransformComponent{}, CameraComponent{});
        }
        Application(const Application&) = delete;
        Application(Application&&) = delete;
        virtual ~Application() {
            ScopedTimer::logResults();
        }
        Application& operator=(const Application&) = delete;
        Application& operator=(Application&&) = delete;
        void run() noexcept {
            static_cast<DerivedType*>(this)->setup();
            auto timeMeasurements = setupTimeMeasurements();
            while (!glfwWindowShouldClose(mWindow.getGLFWWindowPointer())) {
                static_cast<DerivedType*>(this)->update();
                runSystems();
                glfwSwapBuffers(mWindow.getGLFWWindowPointer());
                mInput.nextFrame();
                glfwPollEvents();
                makeTimeMeasurementsStep(timeMeasurements, mTime);
                refreshWindowTitle();
            }
        }

        void quit() noexcept {
            glfwSetWindowShouldClose(mWindow.getGLFWWindowPointer(), true);
        }

    private:
        void runSystems() noexcept {
            runScripts();
            animateSprites();
            renderDynamicSprites();
        }

        void animateSprites() noexcept {
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

        void runScripts() noexcept {
            for (auto&& [entity, scriptComponent] : mRegistry.components<ScriptComponent>()) {
                scriptComponent.script->invokeUpdate(entity);
            }
            for (const auto& command : mAppContext.bufferedScriptCommands) {
                std::visit([this](auto&& typedCommand) { typedCommand.process(mAppContext); }, command);
            }
            mAppContext.bufferedScriptCommands.clear();
        }

        void renderDynamicSprites() noexcept {
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
                    transformMatrix =
                            mRegistry.component<TransformComponent>(current).value().matrix() * transformMatrix;
                }
                mRenderer.drawQuad(transformMatrix, *sprite.shaderProgram, *sprite.texture, sprite.textureRect,
                                   sprite.color);
            }
            mRenderer.endFrame();
        }

        void refreshWindowTitle() noexcept {
            glfwSetWindowTitle(
                    mWindow.getGLFWWindowPointer(),
                    fmt::format("{:.2f} ms ({:.2f} fps)", mTime.meanFrameTime() * 1000.0, mTime.meanFramesPerSecond)
                            .c_str());
        }

    protected:
        Input mInput;
        Window mWindow;
        Registry mRegistry;
        Renderer mRenderer;
        Time mTime;
        AssetDatabase mAssetDatabase;
        Random mRandom;
        ApplicationContext mAppContext;
    };

}// namespace c2k