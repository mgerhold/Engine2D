//
// Created by coder2k on 25.09.2021.
//

#include "Application.hpp"
#include "MathUtils/MathUtils.hpp"

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
        handleParticleEmitters();
        handleParticles();
        renderDynamicSprites();
    }

    void Application::animateSprites() noexcept {
        for (auto&& [entity, animation, dynamicSprite] :
             mRegistry.componentsMutable<SpriteSheetAnimationComponent, DynamicSpriteComponent>()) {
            const double nextFrameChange = animation.lastFrameChange + animation.frameTime;
            if (mTime.elapsed >= nextFrameChange) {
                animation.currentFrame =
                        (animation.currentFrame + 1) % gsl::narrow_cast<int>(animation.spriteSheet->frames.size());
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
        for (auto&& [entity, root, dynamicSprite, transform] :
             mRegistry.components<RootComponent, DynamicSpriteComponent, TransformComponent>()) {
            mRenderer.drawQuad(transform.position, transform.rotation, transform.scale, *(dynamicSprite.shaderProgram),
                               *(dynamicSprite.sprite.texture), dynamicSprite.sprite.textureRect, dynamicSprite.color);
        }
        for (auto&& [entity, relationship, dynamicSprite, transform] :
             mRegistry.components<RelationshipComponent, DynamicSpriteComponent, TransformComponent>()) {
            auto transformMatrix = transform.matrix();
            Entity current = relationship.parent;
            transformMatrix = mRegistry.component<TransformComponent>(current).value().matrix() * transformMatrix;
            while (mRegistry.hasComponent<RelationshipComponent>(current)) {
                current = mRegistry.component<RelationshipComponent>(current)->parent;
                transformMatrix = mRegistry.component<TransformComponent>(current).value().matrix() * transformMatrix;
            }
            mRenderer.drawQuad(transformMatrix, *dynamicSprite.shaderProgram, *dynamicSprite.sprite.texture,
                               dynamicSprite.sprite.textureRect, dynamicSprite.color);
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

    void Application::handleParticleEmitters() noexcept {
        // TODO: remove from local scope to avoid heap allocations every frame
        for (auto&& [entity, particleEmitter, transform, root] :
             mRegistry.componentsMutable<ParticleEmitterComponent, TransformComponent, RootComponent>()) {
            const double spawnInterval = 1.0 / particleEmitter.particleSystem->particlesPerSecond;
            while (mTime.elapsed >= particleEmitter.lastSpawnTime + spawnInterval) {
                mSpawningEmitters.emplace_back(entity);
                particleEmitter.lastSpawnTime += spawnInterval;
            }
        }
        for (auto emitterEntity : mSpawningEmitters) {
            const ParticleSystem& particleSystem =
                    *(mRegistry.component<ParticleEmitterComponent>(emitterEntity).value().particleSystem);
            const glm::vec2 baseScale{ particleSystem.texture->widthToHeightRatio(), 1.0f };
            const auto startScale = baseScale * particleSystem.startScale;
            const auto endScale = baseScale * particleSystem.endScale;
            const double totalLifeTime = particleSystem.startLifeTime + mRandom.range(-particleSystem.lifeTimeVariation,
                                                                                      particleSystem.lifeTimeVariation);
            const float startRotationSpeed = glm::radians(particleSystem.startRotationSpeed +
                                                          mRandom.range(-particleSystem.startRotationSpeedVariation,
                                                                        particleSystem.startRotationSpeedVariation));
            const float endRotationSpeed = glm::radians(
                    particleSystem.endRotationSpeed +
                    mRandom.range(-particleSystem.endRotationSpeedVariation, particleSystem.endRotationSpeedVariation));
            mRegistry.createEntity(
                    TransformComponent{
                            .position{ mRegistry.component<TransformComponent>(emitterEntity).value().position },
                            .rotation{ 0.0f },
                            .scale{ baseScale * particleSystem.startScale } },
                    DynamicSpriteComponent{ .shaderProgram{ particleSystem.shaderProgram },
                                            .sprite{ Sprite::fromTexture(*particleSystem.texture) },
                                            .color{ Color::white() } },
                    RootComponent{},
                    ParticleComponent{ .remainingLifeTime{ totalLifeTime },
                                       .totalLifeTime{ totalLifeTime },
                                       .velocity{ mRandom.unitDirection() * mRandom.range(100.0f, 300.0f) },
                                       .gravity{ particleSystem.gravity },
                                       .startScale{ startScale },
                                       .endScale{ endScale },
                                       .startRotationSpeed{ startRotationSpeed },
                                       .endRotationSpeed{ endRotationSpeed } });
        }
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