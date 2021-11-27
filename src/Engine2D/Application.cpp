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
#include <algorithm>
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
                mRegistry.component<TransformComponent>(mAppContext.mainCameraEntity)->matrix();
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

    Color getGradientColor(const ParticleSystemImpl::ColorGradient& gradient, float interpolationParameter) {
        const auto& marks = gradient.colorGradient;
        if (marks.empty()) {
            return Color{ 0.0f, 0.0f, 0.0f, 1.0f };
        }
        interpolationParameter = std::clamp(interpolationParameter, 0.0f, 1.0f);

        auto upper = std::upper_bound(marks.begin(), marks.end(), interpolationParameter,
                                      [](const float value, const auto& mark) { return value < mark.position; });
        if (upper == marks.end()) {
            --upper;
        }
        auto lower = upper;
        if (lower != marks.begin()) {
            --lower;
        }

        if (upper == lower) {
            return upper->color;
        } else {
            const float distance = upper->position - lower->position;
            const float t = (interpolationParameter - lower->position) / distance;
            return MathUtils::lerp(lower->color, upper->color, t);
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
            const auto spawnInterval = 1.0 / static_cast<double>(getFourWaySelectorValue<float>(
                                                     particleSystem.rateOverTime, mRandom, particleSystem.duration,
                                                     particleSystem.currentDuration));
            if ((particleSystem.currentDuration += mTime.delta) >= particleSystem.duration) {
                if (particleSystem.looping) {
                    particleSystem.currentDuration = std::fmod(particleSystem.currentDuration, particleSystem.duration);
                    particleEmitter.lastSpawnTime = mTime.elapsed - spawnInterval;
                } else {
                    shouldSpawnParticle = false;
                }
            }
            bool foundAny = false;
            while (shouldSpawnParticle && particleSystem.numParticles < particleSystem.maxParticles &&
                   mTime.elapsed >= particleEmitter.lastSpawnTime + spawnInterval) {
                ++particleSystem.numParticles;
                mSpawningEmitters.emplace_back(entity);
                particleEmitter.lastSpawnTime += spawnInterval;
                foundAny = true;
            }
            if (foundAny && particleSystem.numParticles >= particleSystem.maxParticles) {
                // stopped spawning because limit was reached
                const auto timeDistance = mTime.elapsed - particleEmitter.lastSpawnTime - spawnInterval;
                if (timeDistance > 0.0) {
                    const auto steps = std::ceil(timeDistance / spawnInterval);
                    particleEmitter.lastSpawnTime += spawnInterval * steps;
                }
                particleEmitter.lastSpawnTime = mTime.elapsed;
            }
        }
    }

    [[nodiscard]] TransformComponent Application::createParticleTransform(const Entity emitterEntity,
                                                                          const ParticleSystem& particleSystem,
                                                                          Random& random) noexcept {
        const auto emitterPosition = mRegistry.component<TransformComponent>(emitterEntity)->position;
        const auto offsetAngle = glm::radians(random.range(0.0f, particleSystem.emitterArc));
        const auto offsetRadius =
                glm::sqrt(random.range(0.0f, particleSystem.emitterRadius * particleSystem.emitterRadius));
        const auto positionOffset = glm::vec3{ glm::cos(offsetAngle), glm::sin(offsetAngle), 0.0f } * offsetRadius;
        const auto position =
                (particleSystem.simulateInWorldSpace ? emitterPosition : glm::vec3{ 0.0f }) + positionOffset;
        const auto rotationSign = mRandom.sign<float>(1.0f - particleSystem.flipRotation);
        const auto rotationDegrees = getFourWaySelectorValue<float>(
                particleSystem.startRotation, mRandom, particleSystem.duration, particleSystem.currentDuration);
        const auto rotation = glm::radians(rotationDegrees) * rotationSign;
        const auto baseScale = glm::vec2{ particleSystem.sprite.texture->widthToHeightRatio(), 1.0f };
        const auto size = getFourWaySelectorValueVec2(particleSystem.startSize, mRandom, particleSystem.duration,
                                                      particleSystem.currentDuration);
        const auto scale = baseScale * size;
        return TransformComponent{ position, rotation, scale };
    }

    [[nodiscard]] DynamicSpriteComponent Application::createParticleSprite(
            const ParticleSystem& particleSystem) noexcept {
        using namespace c2k::ParticleSystemImpl;// ColorGradient
        const auto color = [&]() {
            if (holds_alternative<Color>(particleSystem.color)) {
                return get<Color>(particleSystem.color);
            } else {
                const auto t = static_cast<float>(particleSystem.currentDuration / particleSystem.duration);
                return getGradientColor(get<ColorGradient>(particleSystem.color), t);
            }
        }();
        return DynamicSpriteComponent{ particleSystem.shaderProgram, particleSystem.sprite, color };
    }

    [[nodiscard]] ParticleComponent Application::createParticle(const Entity particleEmitterEntity,
                                                                const TransformComponent& transform,
                                                                const ParticleSystem& particleSystem) noexcept {
        const auto totalLifeTime = getFourWaySelectorValue<double>(
                particleSystem.startLifetime, mRandom, particleSystem.duration, particleSystem.currentDuration);
        const auto remainingLifeTime = totalLifeTime;
        const auto startSpeed = getFourWaySelectorValue<float>(particleSystem.startSpeed, mRandom,
                                                               particleSystem.duration, particleSystem.currentDuration);
        const auto gravity = glm::vec3{ 0.0f, -9.81f, 0.0f } *
                             getFourWaySelectorValue<float>(particleSystem.gravityModifier, mRandom,
                                                            particleSystem.duration, particleSystem.currentDuration);
        const auto velocityFromGravity = glm::vec3{ 0.0f };
        const auto startScale = transform.scale;
        const auto endScale = transform.scale;
        const float startRotationSpeed = 0.0f;
        const float endRotationSpeed = 0.0f;
        return ParticleComponent{ particleEmitterEntity,
                                  remainingLifeTime,
                                  totalLifeTime,
                                  particleSystem.linearVelocityOverLifetime,
                                  velocityFromGravity,
                                  particleSystem.radialVelocityOverLifetime,
                                  particleSystem.colorOverLifetime,
                                  gravity,
                                  startScale,
                                  endScale };
    }

    void Application::spawnParticles() noexcept {
        for (auto emitterEntity : mSpawningEmitters) {
            const auto& particleSystem = mRegistry.component<ParticleEmitterComponent>(emitterEntity)->particleSystem;
            const auto transform = createParticleTransform(emitterEntity, particleSystem, mRandom);
            const auto dynamicSprite = createParticleSprite(particleSystem);
            const auto particle = createParticle(emitterEntity, transform, particleSystem);
            const auto particleEntity = mRegistry.createEntity(transform, dynamicSprite, particle);
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
        auto entityRange = mRegistry.componentsMutable<ParticleComponent, DynamicSpriteComponent, TransformComponent>();
        std::for_each(std::execution::par, entityRange.begin(), entityRange.end(), [&](auto&& tuple) {
            auto&& [entity, particle, sprite, transform] = tuple;
            const float interpolationParameter =
                    gsl::narrow_cast<float>(1.0 - particle.remainingLifeTime / particle.totalLifeTime);
            const auto delta = gsl::narrow_cast<float>(mTime.delta);
            const auto velocityValue =
                    getFourWaySelectorValueVec2(particle.linearVelocityOverLifetime, mRandom, particle.totalLifeTime,
                                                particle.totalLifeTime - particle.remainingLifeTime);
            const auto velocity = glm::vec3{ velocityValue.x, velocityValue.y, 0.0f };
            particle.velocityFromGravity += particle.gravity * delta;
            transform.position += delta * (velocity + particle.velocityFromGravity);
            transform.scale = MathUtils::lerp(particle.startScale, particle.endScale, interpolationParameter);
            transform.rotation += glm::radians(getFourWaySelectorValue<float>(
                                          particle.radialVelocityOverLifetime, mRandom, particle.totalLifeTime,
                                          particle.totalLifeTime - particle.remainingLifeTime)) *
                                  delta;
            if (particle.colorOverLifetime) {
                sprite.color = getGradientColor(particle.colorOverLifetime.value(), interpolationParameter);
            }
            particle.remainingLifeTime -= mTime.delta;
        });
        for (auto&& [entity, particle, sprite, transform] : entityRange) {
            if (particle.remainingLifeTime < 0.0) {
                mParticleEntitiesToDelete.emplace_back(entity);
            }
        }
        for (auto entityToDelete : mParticleEntitiesToDelete) {
            const auto emitterEntity =
                    mRegistry.componentMutable<ParticleComponent>(entityToDelete)->particleEmitterEntity;
            if (mRegistry.isEntityAlive(emitterEntity)) {
                auto& particleSystem =
                        mRegistry.componentMutable<ParticleEmitterComponent>(emitterEntity)->particleSystem;
                --particleSystem.numParticles;
            }
            mRegistry.destroyEntity(entityToDelete);
        }
        mParticleEntitiesToDelete.clear();
    }

}// namespace c2k