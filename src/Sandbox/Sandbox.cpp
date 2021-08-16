//
// Created by coder2k on 15.05.2021.
//

#include "Sandbox.hpp"
#include "System.hpp"
#include "Image.hpp"
#include "Texture.hpp"
#include "hash/hash.hpp"
#include "ScopedTimer.hpp"
#include "SystemHolder.hpp"
#include <gsl/gsl>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <filesystem>

void Sandbox::setup() noexcept {
#ifdef DEBUG_BUILD
    spdlog::info("This is the debug build");
#else
    spdlog::info("This is the release build");
#endif
    constexpr GUID bjarneID{ 0 };
    mAssetDatabase.loadTexture(std::filesystem::current_path() / "assets" / "images" / "bjarne.jpg", bjarneID);

    constexpr GUID shaderID0{ 1 }, shaderID1{ 2 };
    auto& shader0 = mAssetDatabase.loadShaderProgram(
            std::filesystem::current_path() / "assets" / "shaders" / "default.vert",
            std::filesystem::current_path() / "assets" / "shaders" / "default.frag", shaderID0);
    auto& shader1 = mAssetDatabase.loadShaderProgram(
            std::filesystem::current_path() / "assets" / "shaders" / "default.vert",
            std::filesystem::current_path() / "assets" / "shaders" / "debug.frag", shaderID1);
    glClearColor(73.f / 255.f, 54.f / 255.f, 87.f / 255.f, 1.f);

    // generate game scene
    const auto entity = mRegistry.createEntity();
    mRegistry.attachComponent(entity,
                              Transform{ .position{ 0.0f, 0.0f, 0.0f }, .rotation{ 0.0f }, .scale{ 150.0f, 150.0f } });
    mRegistry.attachComponent(entity, DynamicSprite{ .texture{ &mAssetDatabase.getTexture(bjarneID) },
                                                     .shader{ &mAssetDatabase.getShaderProgram(shaderID0) },
                                                     .color{ 1.0f, 1.0f, 1.0f } });
    const auto cameraEntity = mRegistry.createEntity();
    mRegistry.attachComponent(cameraEntity, Transform{ .position{ 0.0f }, .rotation{ 0.0f }, .scale{ 1.0f } });
    mRegistry.attachComponent(cameraEntity, Camera{});
    const auto& cameraTransform = mRegistry.component<Transform>(cameraEntity).value();
    mRegistry.emplaceSystem<const Transform&, const DynamicSprite&>(
            [this, &shader0, &shader1, &cameraTransform]() {
                const auto framebufferSize = mWindow.getFramebufferSize();
                const auto projectionMatrix =
                        glm::ortho<float>(gsl::narrow_cast<float>(-framebufferSize.width / 2),
                                          gsl::narrow_cast<float>(framebufferSize.width / 2),
                                          gsl::narrow_cast<float>(-framebufferSize.height / 2),
                                          gsl::narrow_cast<float>(framebufferSize.height / 2), -2.0f, 2.0f);
                shader0.setUniform(Hash::staticHashString("projectionMatrix"), projectionMatrix);
                shader1.setUniform(Hash::staticHashString("projectionMatrix"), projectionMatrix);
                mRenderer.clear(true, true);
                mRenderer.beginFrame(Camera::matrix(cameraTransform), projectionMatrix);
            },
            [this]([[maybe_unused]] Entity entity, const Transform& transform, const auto& sprite) {
                mRenderer.drawQuad(transform.position, transform.rotation, transform.scale, *sprite.shader,
                                   *sprite.texture);
            },
            [this]() { mRenderer.endFrame(); });
}

void Sandbox::update() noexcept {
    processInput();
    render();
    mRegistry.runSystems();
}

void Sandbox::processInput() noexcept {
    auto& cameraTransform = std::get<Transform&>(mRegistry.componentsMutable<Camera, Transform>().front());
    constexpr double translationPerSecond{ 100.0 };
    constexpr double zoomFactorPerSecond{ 1.2 };
    constexpr double rotationRadiansPerSecond{ glm::radians(30.0) };
    if (mInput.keyDown(Key::A)) {
        cameraTransform.position.x -= gsl::narrow_cast<float>(translationPerSecond * mTime.delta.count());
    }
    if (mInput.keyDown(Key::D)) {
        cameraTransform.position.x += gsl::narrow_cast<float>(translationPerSecond * mTime.delta.count());
    }
    if (mInput.keyDown(Key::W)) {
        cameraTransform.position.y += gsl::narrow_cast<float>(translationPerSecond * mTime.delta.count());
    }
    if (mInput.keyDown(Key::S)) {
        cameraTransform.position.y -= gsl::narrow_cast<float>(translationPerSecond * mTime.delta.count());
    }
    if (mInput.keyDown(Key::E)) {
        cameraTransform.rotation -= gsl::narrow_cast<float>(rotationRadiansPerSecond * mTime.delta.count());
    }
    if (mInput.keyDown(Key::Q)) {
        cameraTransform.rotation += gsl::narrow_cast<float>(rotationRadiansPerSecond * mTime.delta.count());
    }
    if (mInput.keyDown(Key::NumpadAdd)) {
        cameraTransform.scale /= gsl::narrow_cast<float>((zoomFactorPerSecond - 1.0) * mTime.delta.count() + 1.0);
    }
    if (mInput.keyDown(Key::NumpadSubtract)) {
        cameraTransform.scale *= gsl::narrow_cast<float>((zoomFactorPerSecond - 1.0) * mTime.delta.count() + 1.0);
    }
    if (mInput.mousePressed(MouseButton::Button0)) {
        spdlog::info("Left mouse button pressed");
    }
    if (mInput.mousePressed(MouseButton::Button1)) {
        spdlog::info("Right mouse button pressed");
    }
    if (mInput.mouseDown(MouseButton::Button0)) {
        const auto mousePosition = mInput.mousePosition();
        spdlog::info("Mouse position: ({},{})", mousePosition.x, mousePosition.y);
    }

    if (glfwGetKey(mWindow.getGLFWWindowPointer(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(mWindow.getGLFWWindowPointer(), true);
    }
}

void Sandbox::render() noexcept {
    SCOPED_TIMER();
}
