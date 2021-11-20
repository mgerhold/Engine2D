//
// Created by coder2k on 25.09.2021.
//

#include "Application.hpp"
#include "MathUtils/MathUtils.hpp"
#include "Animation.hpp"
#include "ImGuiUtils/Bezier.hpp"
#include "MathUtils/MathUtils.hpp"
#include "EntityUtils/EntityUtils.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <cmath>
#include <variant>

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
#ifdef DEBUG_BUILD
        spdlog::info("This is the debug build");
#else
        spdlog::info("This is the release build");
#endif
        registerComponentTypes();
        setup();
        auto timeMeasurements = setupTimeMeasurements();
        while (!glfwWindowShouldClose(mWindow.getGLFWWindowPointer())) {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            renderImGui();
            ImGui::Render();

            update();
            runSystems();

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
        handleParticleEmitters();
        handleParticles();
        renderDynamicSprites();
    }

    void Application::animateSprites() noexcept {
        for (auto&& [entity, animation, dynamicSprite] :
             mRegistry.componentsMutable<SpriteSheetAnimationComponent, DynamicSpriteComponent>()) {
            const auto& animationAsset = *animation.animation;
            bool advanceFrame = false;
            double nextFrameChange = 0.0;
            if (holds_alternative<double>(animationAsset)) {
                // the animation only holds information of the framerate
                nextFrameChange = animation.lastFrameChange + 1.0 / get<double>(animationAsset);
                advanceFrame = mTime.elapsed >= nextFrameChange;
                if (advanceFrame) {
                    animation.currentFrame =
                            (animation.currentFrame + 1) % gsl::narrow_cast<int>(animation.spriteSheet->frames.size());
                }
            } else if (holds_alternative<std::vector<double>>(animationAsset)) {
                // the animation holds the frametimes for all needed frames seperately
                const auto& frameTimes = get<std::vector<double>>(animationAsset);
                nextFrameChange = animation.lastFrameChange + frameTimes[animation.currentFrame];
                advanceFrame = mTime.elapsed >= nextFrameChange;
                if (advanceFrame) {
                    animation.currentFrame =
                            (animation.currentFrame + 1) %
                            gsl::narrow_cast<int>(std::min(animation.spriteSheet->frames.size(), frameTimes.size()));
                }
            } else {
                assert(!"invalid animation type");
            }
            if (advanceFrame) {
                dynamicSprite.sprite.textureRect = animation.spriteSheet->frames[animation.currentFrame].rect;
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
        for (auto&& [entity, dynamicSprite, transform] :
             mRegistry.components<DynamicSpriteComponent, TransformComponent>()) {
            mRenderer.drawQuad(EntityUtils::getGlobalTransform(mRegistry, entity), *dynamicSprite.shaderProgram,
                               *dynamicSprite.sprite.texture, dynamicSprite.sprite.textureRect, dynamicSprite.color);
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

    void Application::registerComponentTypes() noexcept {
        mRegistry.registerType<RootComponent>();
        mRegistry.registerType<RelationshipComponent>();
        mRegistry.registerType<TransformComponent>();
        mRegistry.registerType<DynamicSpriteComponent>();
        mRegistry.registerType<SpriteSheetAnimationComponent>();
        mRegistry.registerType<CameraComponent>();
        mRegistry.registerType<ScriptComponent>();
        mRegistry.registerType<ParticleEmitterComponent>();
        mRegistry.registerType<ParticleComponent>();
    }

    template<typename T>
    T getTwoWaySelectorValue(const auto& variant, Random& random) noexcept {
        using namespace c2k::ParticleSystemImpl;
        if (holds_alternative<T>(variant)) {
            return get<T>(variant);
        }
        const auto& range = get<Range<T>>(variant);
        return random.range(std::min(range.min, range.max), std::max(range.min, range.max));
    }

    template<typename T>
    T getFourWaySelectorValue(const auto& variant,
                              Random& random,
                              double particleSystemDuration,
                              double particleSystemCurrentDuration) noexcept {
        using namespace c2k::ParticleSystemImpl;
        if (holds_alternative<T>(variant)) {
            return get<T>(variant);
        } else if (holds_alternative<Range<T>>(variant)) {
            const auto min = get<Range<T>>(variant).min;
            const auto max = get<Range<T>>(variant).max;
            return random.range(std::min(min, max), std::max(min, max));
        } else if (holds_alternative<BezierCurve>(variant)) {
            const auto interpolationParameter =
                    gsl::narrow_cast<float>(particleSystemCurrentDuration / particleSystemDuration);
            const auto& curve = get<BezierCurve>(variant);
            return gsl::narrow_cast<T>(ImGui::BezierValue(interpolationParameter, curve));
        } else {
            assert(false);
            return T{ 0 };
        }
    }

    glm::vec2 getFourWaySelectorValueVec2(const auto& variant,
                                          Random& random,
                                          double particleSystemDuration,
                                          double particleSystemCurrentDuration) noexcept {
        using namespace c2k::ParticleSystemImpl;
        if (holds_alternative<glm::vec2>(variant)) {
            return get<glm::vec2>(variant);
        } else if (holds_alternative<Range<glm::vec2>>(variant)) {
            const auto interpolationParameter = random.get<float>();
            const auto min = get<Range<glm::vec2>>(variant).min;
            const auto max = get<Range<glm::vec2>>(variant).max;
            return MathUtils::lerp(min, max, interpolationParameter);
        } else if (holds_alternative<BezierCurves2D>(variant)) {
            const auto interpolationParameter =
                    gsl::narrow_cast<float>(particleSystemCurrentDuration / particleSystemDuration);
            const auto& curves = get<BezierCurves2D>(variant);
            return glm::vec2{ ImGui::BezierValue(interpolationParameter, curves.x),
                              ImGui::BezierValue(interpolationParameter, curves.y) };
        } else {
            assert(false);
            return glm::vec2{ 0.0f };
        }
    }

    void Application::collectSpawningParticleEmitters() noexcept {
        using namespace c2k::ParticleSystemImpl;
        /* find all the particle emitters that should spawn at least one particle within the
           current frame and save them into mSpawningEmitters (they are saved multiple times
           if they should spawn more than one particle */
        for (auto&& [entity, particleEmitter, transform, root] :
             mRegistry.componentsMutable<ParticleEmitterComponent, TransformComponent, RootComponent>()) {
            auto& particleSystem = particleEmitter.particleSystem;
            const double startDelay = getTwoWaySelectorValue<double>(particleSystem.startDelay, mRandom);
            bool shouldSpawnParticle = particleSystem.currentDuration >= startDelay;
            if ((particleSystem.currentDuration += mTime.delta) >= particleSystem.duration) {
                if (particleSystem.looping) {
                    particleSystem.currentDuration = std::fmod(particleSystem.currentDuration, particleSystem.duration);
                    particleEmitter.lastSpawnTime = mTime.elapsed - particleSystem.spawnInterval();
                } else {
                    shouldSpawnParticle = false;
                }
            }
            while (shouldSpawnParticle &&
                   mTime.elapsed >= particleEmitter.lastSpawnTime + particleSystem.spawnInterval()) {
                mSpawningEmitters.emplace_back(entity);
                particleEmitter.lastSpawnTime += particleSystem.spawnInterval();
            }
        }
    }

    void Application::spawnParticles() noexcept {
        // iterate mSpawningEmitters to spawn all new particles for the current frame
        for (auto emitterEntity : mSpawningEmitters) {
            const ParticleSystem& particleSystem =
                    mRegistry.component<ParticleEmitterComponent>(emitterEntity).value().particleSystem;
            const glm::vec2 baseScale{ particleSystem.sprite.texture->widthToHeightRatio(), 1.0f };
            const auto startScale =
                    baseScale * getFourWaySelectorValueVec2(particleSystem.startSize, mRandom, particleSystem.duration,
                                                            particleSystem.currentDuration);
            const auto endScale = startScale;
            const double totalLifeTime = getFourWaySelectorValue<double>(
                    particleSystem.startLifetime, mRandom, particleSystem.duration, particleSystem.currentDuration);
            const float startRotationSpeed = 0.0f;
            const float endRotationSpeed = 0.0f;
            const auto startSpeed = getFourWaySelectorValue<float>(
                    particleSystem.startSpeed, mRandom, particleSystem.duration, particleSystem.currentDuration);
            const auto particlePosition =
                    (particleSystem.simulateInWorldSpace
                             ? mRegistry.component<TransformComponent>(emitterEntity).value().position
                             : glm::vec3{ 0.0f });
            const auto particleEntity = mRegistry.createEntity(
                    TransformComponent{ .position{ particlePosition }, .rotation{ 0.0f }, .scale{ startScale } },
                    DynamicSpriteComponent{ .shaderProgram{ particleSystem.shaderProgram },
                                            .sprite{ particleSystem.sprite },
                                            .color{ Color::white() } },
                    ParticleComponent{
                            .remainingLifeTime{ totalLifeTime },
                            .totalLifeTime{ totalLifeTime },
                            .velocity{ mRandom.unitDirection() * startSpeed },
                            // TODO: handle gravity correctly
                            .gravity{ glm::vec3{ 0.0f, -9.81f, 0.0f } * get<float>(particleSystem.gravityModifier) },
                            .startScale{ startScale },
                            .endScale{ endScale },
                            .startRotationSpeed{ startRotationSpeed },
                            .endRotationSpeed{ endRotationSpeed } });
            if (particleSystem.simulateInWorldSpace) {
                mRegistry.attachComponent(particleEntity, RootComponent{});
            } else {
                mRegistry.attachComponent(particleEntity, RelationshipComponent{ .parent{ emitterEntity } });
            }
        }
    }

    void Application::handleParticleEmitters() noexcept {
        collectSpawningParticleEmitters();
        spawnParticles();
        mSpawningEmitters.clear();
    }

    void Application::handleParticles() noexcept {
        for (auto&& [entity, particle, transform] :
             mRegistry.componentsMutable<ParticleComponent, TransformComponent>()) {
            const float interpolationParameter =
                    gsl::narrow_cast<float>(1.0 - particle.remainingLifeTime / particle.totalLifeTime);
            const auto delta = gsl::narrow_cast<float>(mTime.delta);
            particle.velocity += particle.gravity * delta;
            transform.position += particle.velocity * delta;
            transform.scale = MathUtils::lerp(particle.startScale, particle.endScale, interpolationParameter);
            transform.rotation +=
                    MathUtils::lerp(particle.startRotationSpeed, particle.endRotationSpeed, interpolationParameter) *
                    delta;
            particle.remainingLifeTime -= mTime.delta;
            if (particle.remainingLifeTime < 0.0) {
                mParticleEntitiesToDelete.emplace_back(entity);
            }
        }
        for (auto entityToDelete : mParticleEntitiesToDelete) {
            mRegistry.destroyEntity(entityToDelete);
        }
        mParticleEntitiesToDelete.clear();
    }

}// namespace c2k