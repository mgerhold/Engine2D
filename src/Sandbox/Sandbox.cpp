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
#include "AssetList.hpp"
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
    mAssetDatabase.loadFromList(AssetDatabase::assetPath() / "scenes" / "assets.json");

    const auto textureGUID{ GUID::fromString("9043b452-363c-4917-bfde-592a72077e37") };
    const auto shaderGUID{ GUID::fromString("b520f0eb-1756-41e0-ac07-66c3338bc594") };
    mRenderer.setClearColor({ 73, 54, 87 });

    // generate game scene
    constexpr float textureHeight = 40.0f;
    const glm::vec2 textureSize{ textureHeight * mAssetDatabase.texture(textureGUID).widthToHeightRatio(),
                                 textureHeight };
    mRegistry.createEntity(Transform{ .position{ 0.0f, 0.0f, 0.0f }, .rotation{ 0.0f }, .scale{ textureSize } },
                           DynamicSprite{ .texture{ &mAssetDatabase.texture(textureGUID) },
                                          .shader{ &mAssetDatabase.shaderProgramMutable(shaderGUID) },
                                          .color{ 255, 40, 160 } });
    std::random_device randomDevice;
    std::mt19937 randomEngine{ randomDevice() };
    std::uniform_int_distribution distribution{ -2000, 2000 };
    constexpr int numEntities = 500;
    for (auto _ : ranges::views::ints(0, numEntities)) {
        const glm::vec3 position{ distribution(randomEngine), distribution(randomEngine), 0.0f };
        mRegistry.createEntity(Transform{ .position{ position }, .rotation{ 0.0f }, .scale{ textureSize } },
                               DynamicSprite{ .texture{ &mAssetDatabase.texture(textureGUID) },
                                              .shader{ &mAssetDatabase.shaderProgramMutable(shaderGUID) },
                                              .color{ Color::white() } });
    }
    const auto cameraEntity =
            mRegistry.createEntity(Transform{ .position{ 0.0f }, .rotation{ 0.0f }, .scale{ 1.0f } }, Camera{});
    const auto& cameraTransform = mRegistry.component<Transform>(cameraEntity).value();
    mRegistry.emplaceSystem<const DynamicSprite&, const Transform&>(
            [this, &cameraTransform]() {
                mRenderer.clear(true, true);
                mRenderer.beginFrame(Camera::viewMatrix(cameraTransform),
                                     Camera::projectionMatrix(mWindow.framebufferSize()));
            },
            [this]([[maybe_unused]] Entity entity, const auto& sprite, const Transform& transform) {
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
