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
#include "Component.hpp"

namespace c2k {

    void Sandbox::setup() noexcept {
#ifdef DEBUG_BUILD
        spdlog::info("This is the debug build");
#else
        spdlog::info("This is the release build");
#endif
        mAssetDatabase.loadFromList(AssetDatabase::assetPath() / "scenes" / "assets.json");

        const auto spriteSheetGUID{ GUID::fromString("c15111ea-7ba8-4e65-8f24-40c868498d5b") };
        const auto fireTextureGUID{ GUID::fromString("c22764c9-9750-4749-810e-10f4c6f50123") };
        const auto textureGUID{ GUID::fromString("9043b452-363c-4917-bfde-592a72077e37") };
        const auto shaderGUID{ GUID::fromString("b520f0eb-1756-41e0-ac07-66c3338bc594") };

        mAssetDatabase.textureMutable(fireTextureGUID).setFiltering(Texture::Filtering::Nearest);

        // generate game scene
        constexpr float textureHeight = 40.0f;
        const glm::vec2 textureSize{ textureHeight * mAssetDatabase.texture(textureGUID).widthToHeightRatio(),
                                     textureHeight };
        const glm::vec2 fireTextureSize{
            textureHeight * mAssetDatabase.spriteSheet(spriteSheetGUID).frames[0].getWidthToHeightRatio(), textureHeight
        };
        const auto anchor = mRegistry.createEntity(c2k::Components::Transform{}, c2k::Components::RootComponent{});
        mRegistry.createEntity(
                Components::Transform{ .scale{ fireTextureSize * 2.0f } },
                Components::DynamicSprite{
                        .textureRect{ mAssetDatabase.spriteSheet(spriteSheetGUID).frames[0].rect },
                        .color{ Color::white() },
                        .texture{ mAssetDatabase.spriteSheet(spriteSheetGUID).texture },
                        .shader{ &mAssetDatabase.shaderProgramMutable(shaderGUID) },
                },
                Components::SpriteSheetAnimation{ .spriteSheet{ &mAssetDatabase.spriteSheet(spriteSheetGUID) },
                                                  .lastFrameChange{ mTime.elapsed.count() },
                                                  .frameTime{ 1.0 / 50.0 },
                                                  .currentFrame{ 0 } },
                Components::Relationship{ .parent{ anchor } });
        const auto secondFlame = mRegistry.createEntity(
                Components::Transform{ .position{ 200.0f, 0.0f, 0.0f }, .scale{ fireTextureSize } },
                Components::DynamicSprite{
                        .textureRect{ mAssetDatabase.spriteSheet(spriteSheetGUID).frames[0].rect },
                        .color{ Color::white() },
                        .texture{ mAssetDatabase.spriteSheet(spriteSheetGUID).texture },
                        .shader{ &mAssetDatabase.shaderProgramMutable(shaderGUID) },
                },
                Components::SpriteSheetAnimation{ .spriteSheet{ &mAssetDatabase.spriteSheet(spriteSheetGUID) },
                                                  .lastFrameChange{ mTime.elapsed.count() },
                                                  .frameTime{ 1.0 / 50.0 },
                                                  .currentFrame{ 0 } },
                Components::Relationship{ .parent{ anchor } });
        mRegistry.createEntity(
                Components::Transform{ .position{ 5.0f, 0.0f, 0.0f }, .scale{ 0.5f, 0.5f } },
                Components::DynamicSprite{
                        .textureRect{ mAssetDatabase.spriteSheet(spriteSheetGUID).frames[0].rect },
                        .color{ Color::white() },
                        .texture{ mAssetDatabase.spriteSheet(spriteSheetGUID).texture },
                        .shader{ &mAssetDatabase.shaderProgramMutable(shaderGUID) },
                },
                Components::SpriteSheetAnimation{ .spriteSheet{ &mAssetDatabase.spriteSheet(spriteSheetGUID) },
                                                  .lastFrameChange{ mTime.elapsed.count() },
                                                  .frameTime{ 1.0 / 50.0 },
                                                  .currentFrame{ 0 } },
                Components::Relationship{ .parent{ secondFlame } });

        constexpr int numEntities = 500;
        for ([[maybe_unused]] auto _ : ranges::views::ints(0, numEntities)) {
            const glm::vec3 position{ mRandom.range(-2000.0f, 2000.0f), mRandom.range(-2000.0f, 2000.0f), 0.0f };
            mRegistry.createEntity(Components::Transform{ .position{ position }, .scale{ textureSize } },
                                   Components::DynamicSprite{
                                           .textureRect{ Rect::unit() },
                                           .color{ Color::white() },
                                           .texture{ &mAssetDatabase.texture(textureGUID) },
                                           .shader{ &mAssetDatabase.shaderProgramMutable(shaderGUID) },
                                   },
                                   Components::RootComponent{});
        }
        mRegistry.createEntity(Components::Script{
                .script = &mAssetDatabase.scriptMutable(GUID::fromString("5874d6e9-5529-45bc-829b-d6002ef21d70")) });
        mRegistry.createEntity(Components::Script{
                .script = &mAssetDatabase.scriptMutable(GUID::fromString("88cc0c19-cda5-4633-9e25-70635437a368")) });
        const auto cameraEntity = mRegistry.createEntity(Components::Transform{}, Components::Camera{});
        mRegistry.emplaceSystem<Components::DynamicSprite&, Components::SpriteSheetAnimation&>(
                []() {},
                [this](Entity, Components::DynamicSprite& sprite, Components::SpriteSheetAnimation& animation) {
                    const double nextFrameChange = animation.lastFrameChange + animation.frameTime;
                    if (mTime.elapsed.count() >= nextFrameChange) {
                        animation.currentFrame = (animation.currentFrame + 1) %
                                                 gsl::narrow_cast<int>(animation.spriteSheet->frames.size());
                        sprite.textureRect = animation.spriteSheet->frames[animation.currentFrame].rect;
                        animation.lastFrameChange = nextFrameChange;
                    }
                },
                []() {});
        mRegistry.emplaceSystem<>([]() {},
                                  [this, anchor, secondFlame]() {
                                      mRegistry.componentMutable<Components::Transform>(anchor).value().rotation +=
                                              gsl::narrow_cast<float>(glm::radians(20.0f) * mTime.delta.count());
                                      mRegistry.componentMutable<Components::Transform>(secondFlame).value().rotation +=
                                              gsl::narrow_cast<float>(glm::radians(20.0f) * mTime.delta.count());
                                  },
                                  []() {});
        /*mRegistry.emplaceSystem<const DynamicSprite&, Transform&>(
                []() {},
                [this](Entity, const DynamicSprite&, Transform& transform) {
                    transform.rotation += gsl::narrow_cast<float>(glm::radians(33.0f) * mTime.delta.count());
                },
                []() {});*/

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
                    auto& cameraTransform = std::get<Components::Transform&>(
                            mRegistry.componentsMutable<Components::Camera, Components::Transform>().front());
                    if (mInput.keyDown(Key::A)) {
                        cameraTransform.position.x -=
                                gsl::narrow_cast<float>(translationPerSecond * mTime.delta.count());
                    }
                    if (mInput.keyDown(Key::D)) {
                        cameraTransform.position.x +=
                                gsl::narrow_cast<float>(translationPerSecond * mTime.delta.count());
                    }
                    if (mInput.keyDown(Key::W)) {
                        cameraTransform.position.y +=
                                gsl::narrow_cast<float>(translationPerSecond * mTime.delta.count());
                    }
                    if (mInput.keyDown(Key::S)) {
                        cameraTransform.position.y -=
                                gsl::narrow_cast<float>(translationPerSecond * mTime.delta.count());
                    }
                    if (mInput.keyDown(Key::E)) {
                        cameraTransform.rotation -=
                                gsl::narrow_cast<float>(rotationRadiansPerSecond * mTime.delta.count());
                    }
                    if (mInput.keyDown(Key::Q)) {
                        cameraTransform.rotation +=
                                gsl::narrow_cast<float>(rotationRadiansPerSecond * mTime.delta.count());
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
                [this, anchor]() {
                    /* this system sets the position of the first sprite to the current
                 * position of the mouse every frame */
                    /*auto& transform =
                            std::get<Transform&>(mRegistry.componentsMutable<DynamicSprite, Transform>().front());*/
                    auto& cameraTransform = std::get<Components::Transform&>(
                            mRegistry.componentsMutable<Components::Camera, Components::Transform>().front());

                    auto& transform = mRegistry.componentMutable<Components::Transform>(anchor).value();
                    const auto mousePosition = mInput.mousePosition();
                    const auto worldPosition = Components::Camera::screenToWorldPoint(mousePosition, cameraTransform);
                    transform.position.x = worldPosition.x;
                    transform.position.y = worldPosition.y;
                });
        mRegistry.emplaceSystem<const Components::Script&>(
                []() {},
                [this](Entity entity, const Components::Script& scriptComponent) {
                    scriptComponent.script->invoke("update", entity);
                },
                []() {});
        mRegistry.addScreenClearer(mAppContext, true, true);
        mRegistry.addDynamicSpriteRenderer(mAppContext,
                                           mRegistry.component<Components::Transform>(cameraEntity).value());
        mRegistry.addDynamicSpriteRenderer(mAppContext);// overlay
    }

    void Sandbox::update() noexcept { }

}// namespace c2k