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

    // generate game scene
    constexpr float textureHeight = 40.0f;
    const glm::vec2 textureSize{ textureHeight * mAssetDatabase.texture(textureGUID).widthToHeightRatio(),
                                 textureHeight };
    mRegistry.createEntity(Transform{ .scale{ textureSize } },
                           DynamicSprite{ .texture{ &mAssetDatabase.texture(textureGUID) },
                                          .shader{ &mAssetDatabase.shaderProgramMutable(shaderGUID) },
                                          .color{ 255, 40, 160 } });
    constexpr int numEntities = 500;
    for (auto _ : ranges::views::ints(0, numEntities)) {
        const glm::vec3 position{ mRandom.range(-2000.0f, 2000.0f), mRandom.range(-2000.0f, 2000.0f), 0.0f };
        mRegistry.createEntity(Transform{ .position{ position }, .scale{ textureSize } },
                               DynamicSprite{ .texture{ &mAssetDatabase.texture(textureGUID) },
                                              .shader{ &mAssetDatabase.shaderProgramMutable(shaderGUID) },
                                              .color{ Color::white() } });
    }
    const auto cameraEntity = mRegistry.createEntity(Transform{}, Camera{});
    mRegistry.emplaceSystem<Transform&, const DynamicSprite&>(
            []() {},
            [this](Entity, Transform& transform, const DynamicSprite&) {
                transform.rotation += gsl::narrow_cast<float>(glm::radians(20.0f) * mTime.delta.count());
            },
            []() {});

    mRegistry.emplaceSystem<>([]() {},
                              [this]() {
                                  /* this system quits the application when the escape key
                                   * is pressed */
                                  if (mInput.keyPressed(Key::Escape)) {
                                      quit();
                                  }
                              });
    mRegistry.emplaceSystem<>(
            []() {},
            [this]() {
                /* this system handles controlling the camera */
                constexpr double translationPerSecond{ 100.0 };
                constexpr double zoomFactorPerSecond{ 1.2 };
                constexpr double rotationRadiansPerSecond{ glm::radians(30.0) };
                auto& cameraTransform = std::get<Transform&>(mRegistry.componentsMutable<Camera, Transform>().front());
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
                    cameraTransform.scale /=
                            gsl::narrow_cast<float>((zoomFactorPerSecond - 1.0) * mTime.delta.count() + 1.0);
                }
                if (mInput.keyDown(Key::NumpadSubtract)) {
                    cameraTransform.scale *=
                            gsl::narrow_cast<float>((zoomFactorPerSecond - 1.0) * mTime.delta.count() + 1.0);
                }
            });
    mRegistry.emplaceSystem<>(
            []() {},
            [this]() {
                /* this system sets the position of the first sprite to the current
                 * position of the mouse every frame */
                auto& transform = std::get<Transform&>(mRegistry.componentsMutable<DynamicSprite, Transform>().front());
                auto& cameraTransform = std::get<Transform&>(mRegistry.componentsMutable<Camera, Transform>().front());

                const auto mousePosition = mInput.mousePosition();
                const auto worldPosition = Camera::screenToWorldPoint(mousePosition, cameraTransform);
                transform.position.x = worldPosition.x;
                transform.position.y = worldPosition.y;
            });
    mRegistry.addScreenClearer(mRenderer, true, true);
    mRegistry.addDynamicSpriteRenderer(mRenderer, mRegistry.component<Transform>(cameraEntity).value());
    mRegistry.addDynamicSpriteRenderer(mRenderer);// overlay
}

void Sandbox::update() noexcept { }