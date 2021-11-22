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

    class Application {
    public:
        Application(const std::string& title = "c2k application",
                    WindowSize size = WindowSize{ .width{ 800 }, .height{ 600 } },
                    OpenGLVersion version = OpenGLVersion{ .major{ 4 }, .minor{ 5 } }) noexcept;
        Application(const Application&) = delete;
        Application(Application&&) = delete;
        virtual ~Application() noexcept;
        Application& operator=(const Application&) = delete;
        Application& operator=(Application&&) = delete;
        void run() noexcept;
        void quit() noexcept;

    private:
        virtual void setup() noexcept = 0;
        virtual void update() noexcept = 0;
        virtual void renderImGui() noexcept { }
        void runSystems() noexcept;
        void animateSprites() noexcept;
        void runScripts() noexcept;
        void renderDynamicSprites() noexcept;
        void collectSpawningParticleEmitters() noexcept;
        [[nodiscard]] TransformComponent createParticleTransform(Entity emitterEntity,
                                                                 const ParticleSystem& particleSystem) noexcept;
        [[nodiscard]] DynamicSpriteComponent createParticleSprite(const ParticleSystem& particleSystem) noexcept;
        [[nodiscard]] ParticleComponent createParticle(const TransformComponent& transform,
                                                       const ParticleSystem& particleSystem) noexcept;
        void spawnParticles() noexcept;
        void handleParticleEmitters() noexcept;
        void handleParticles() noexcept;
        void refreshWindowTitle() noexcept;
        void registerComponentTypes() noexcept;

    protected:
        Input mInput;
        Window mWindow;
        Registry mRegistry;
        Renderer mRenderer;
        Time mTime;
        AssetDatabase mAssetDatabase;
        Random mRandom;
        ApplicationContext mAppContext;

    private:
        std::vector<Entity> mSpawningEmitters;
        std::vector<Entity> mParticleEntitiesToDelete;
    };

}// namespace c2k