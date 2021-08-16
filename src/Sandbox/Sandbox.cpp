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
#include <random>
#include <filesystem>

void Sandbox::setup() noexcept {
#ifdef DEBUG_BUILD
    spdlog::info("This is the debug build");
#else
    spdlog::info("This is the release build");
#endif
    constexpr GUID bjarneID{ 0 };
    auto& texture =
            mAssetDatabase.loadTexture(std::filesystem::current_path() / "assets" / "images" / "bjarne.jpg", bjarneID);

    constexpr GUID shaderID0{ 1 }, shaderID1{ 2 };
    auto& shader0 = mAssetDatabase.loadShaderProgram(
            std::filesystem::current_path() / "assets" / "shaders" / "default.vert",
            std::filesystem::current_path() / "assets" / "shaders" / "default.frag", shaderID0);
    auto& shader1 = mAssetDatabase.loadShaderProgram(
            std::filesystem::current_path() / "assets" / "shaders" / "default.vert",
            std::filesystem::current_path() / "assets" / "shaders" / "debug.frag", shaderID1);
    mRenderer.setClearColor({ 73, 54, 87 });

    // generate game scene
    constexpr float textureHeight = 40.0f;
    mRegistry.createEntity(Transform{ .position{ 0.0f, 0.0f, 0.0f },
                                      .rotation{ 0.0f },
                                      .scale{ textureHeight * texture.widthToHeightRatio(), textureHeight } },
                           DynamicSprite{ .texture{ &mAssetDatabase.getTexture(bjarneID) },
                                          .shader{ &mAssetDatabase.getShaderProgram(shaderID0) },
                                          .color{ 255, 40, 160 } });
    std::random_device randomDevice;
    std::mt19937 randomEngine{ randomDevice() };
    std::uniform_int_distribution distribution{ -2000, 2000 };
    constexpr int numEntities = 500;
    for (auto _ : ranges::views::ints(0, numEntities)) {
        const glm::vec3 position{ distribution(randomEngine), distribution(randomEngine), 0.0f };
        const glm::vec2 scale{ texture.widthToHeightRatio() * textureHeight, textureHeight };
        mRegistry.createEntity(Transform{ .position{ position }, .rotation{ 0.0f }, .scale{ scale } },
                               DynamicSprite{ .texture{ &mAssetDatabase.getTexture(bjarneID) },
                                              .shader{ &mAssetDatabase.getShaderProgram(shaderID0) },
                                              .color{ Color::white() } });
    }
    const auto cameraEntity =
            mRegistry.createEntity(Transform{ .position{ 0.0f }, .rotation{ 0.0f }, .scale{ 1.0f } }, Camera{});
    const auto& cameraTransform = mRegistry.component<Transform>(cameraEntity).value();
    mRegistry.emplaceSystem<const Transform&, const DynamicSprite&>(
            [this, &cameraTransform]() {
                mRenderer.clear(true, true);
                mRenderer.beginFrame(Camera::viewMatrix(cameraTransform),
                                     Camera::projectionMatrix(mWindow.framebufferSize()));
            },
            [this]([[maybe_unused]] Entity entity, const Transform& transform, const auto& sprite) {
                mRenderer.drawQuad(transform.position, transform.rotation, transform.scale, *sprite.shader,
                                   *sprite.texture, sprite.color);
            },
            [this]() { mRenderer.endFrame(); });
}

void Sandbox::update() noexcept {
    processInput();
    render();
    mRegistry.runSystems();
}

void Sandbox::processInput() noexcept {
    const auto& cameraEntity = mRegistry.componentsMutable<Camera, Transform>().front();
    auto& cameraTransform = std::get<Transform&>(cameraEntity);
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
    auto& textureTransform = std::get<Transform&>(mRegistry.componentsMutable<DynamicSprite, Transform>().front());
    const auto mousePosition = mInput.mousePosition();
    const auto worldPosition = Camera::screenToWorldPoint(mousePosition, cameraTransform);
    textureTransform.position.x = worldPosition.x;
    textureTransform.position.y = worldPosition.y;

    if (glfwGetKey(mWindow.getGLFWWindowPointer(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(mWindow.getGLFWWindowPointer(), true);
    }
}

void Sandbox::render() noexcept {
    SCOPED_TIMER();
}
