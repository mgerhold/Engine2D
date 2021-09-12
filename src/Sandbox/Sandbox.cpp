//
// Created by coder2k on 15.05.2021.
//

#include "Sandbox.hpp"
#include "Image.hpp"
#include "Texture.hpp"
#include "hash/hash.hpp"
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
        const auto anchor = mRegistry.createEntity(TransformComponent{}, RootComponent{});
        mRegistry.createEntity(
                TransformComponent{ .scale{ fireTextureSize * 2.0f } },
                DynamicSpriteComponent{
                        .textureRect{ mAssetDatabase.spriteSheet(spriteSheetGUID).frames[0].rect },
                        .color{ Color::white() },
                        .texture{ mAssetDatabase.spriteSheet(spriteSheetGUID).texture },
                        .shaderProgram{ &mAssetDatabase.shaderProgramMutable(shaderGUID) },
                },
                SpriteSheetAnimationComponent{ .spriteSheet{ &mAssetDatabase.spriteSheet(spriteSheetGUID) },
                                               .lastFrameChange{ mTime.elapsed },
                                               .frameTime{ 1.0 / 50.0 },
                                               .currentFrame{ 0 } },
                RelationshipComponent{ .parent{ anchor } });
        const auto secondFlame = mRegistry.createEntity(
                TransformComponent{ .position{ 200.0f, 0.0f, 0.0f }, .scale{ fireTextureSize } },
                DynamicSpriteComponent{
                        .textureRect{ mAssetDatabase.spriteSheet(spriteSheetGUID).frames[0].rect },
                        .color{ Color::white() },
                        .texture{ mAssetDatabase.spriteSheet(spriteSheetGUID).texture },
                        .shaderProgram{ &mAssetDatabase.shaderProgramMutable(shaderGUID) },
                },
                SpriteSheetAnimationComponent{ .spriteSheet{ &mAssetDatabase.spriteSheet(spriteSheetGUID) },
                                               .lastFrameChange{ mTime.elapsed },
                                               .frameTime{ 1.0 / 50.0 },
                                               .currentFrame{ 0 } },
                RelationshipComponent{ .parent{ anchor } });
        mRegistry.createEntity(
                TransformComponent{ .position{ 5.0f, 0.0f, 0.0f }, .scale{ 0.5f, 0.5f } },
                DynamicSpriteComponent{
                        .textureRect{ mAssetDatabase.spriteSheet(spriteSheetGUID).frames[0].rect },
                        .color{ Color::white() },
                        .texture{ mAssetDatabase.spriteSheet(spriteSheetGUID).texture },
                        .shaderProgram{ &mAssetDatabase.shaderProgramMutable(shaderGUID) },
                },
                SpriteSheetAnimationComponent{ .spriteSheet{ &mAssetDatabase.spriteSheet(spriteSheetGUID) },
                                               .lastFrameChange{ mTime.elapsed },
                                               .frameTime{ 1.0 / 50.0 },
                                               .currentFrame{ 0 } },
                RelationshipComponent{ .parent{ secondFlame } });

        constexpr int numEntities = 100;
        for ([[maybe_unused]] auto _ : ranges::views::ints(0, numEntities)) {
            const glm::vec3 position{ mRandom.range(-2000.0f, 2000.0f), mRandom.range(-2000.0f, 2000.0f), 0.0f };
            mRegistry.createEntity(TransformComponent{ .position{ position }, .scale{ textureSize } },
                                   DynamicSpriteComponent{
                                           .textureRect{ Rect::unit() },
                                           .color{ Color::white() },
                                           .texture{ &mAssetDatabase.texture(textureGUID) },
                                           .shaderProgram{ &mAssetDatabase.shaderProgramMutable(shaderGUID) },
                                   },
                                   RootComponent{},
                                   ScriptComponent{ .script = &mAssetDatabase.scriptMutable(GUID::fromString(
                                                            "5874d6e9-5529-45bc-829b-d6002ef21d70")) });
        }
        const auto cameraEntity = mRegistry.createEntity(TransformComponent{}, CameraComponent{});
        mRegistry.emplaceSystem<DynamicSpriteComponent&, SpriteSheetAnimationComponent&>(
                []() {},
                [this](Entity, DynamicSpriteComponent& sprite, SpriteSheetAnimationComponent& animation) {
                    const double nextFrameChange = animation.lastFrameChange + animation.frameTime;
                    if (mTime.elapsed >= nextFrameChange) {
                        animation.currentFrame = (animation.currentFrame + 1) %
                                                 gsl::narrow_cast<int>(animation.spriteSheet->frames.size());
                        sprite.textureRect = animation.spriteSheet->frames[animation.currentFrame].rect;
                        animation.lastFrameChange = nextFrameChange;
                    }
                },
                []() {});
        mRegistry.emplaceSystem<>([]() {},
                                  [this, anchor, secondFlame]() {
                                      mRegistry.componentMutable<TransformComponent>(anchor).value().rotation +=
                                              gsl::narrow_cast<float>(glm::radians(20.0f) * mTime.delta);
                                      mRegistry.componentMutable<TransformComponent>(secondFlame).value().rotation +=
                                              gsl::narrow_cast<float>(glm::radians(20.0f) * mTime.delta);
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
                    auto& cameraTransform = std::get<TransformComponent&>(
                            mRegistry.componentsMutable<CameraComponent, TransformComponent>().front());
                    if (mInput.keyDown(Key::A)) {
                        cameraTransform.position.x -= gsl::narrow_cast<float>(translationPerSecond * mTime.delta);
                    }
                    if (mInput.keyDown(Key::D)) {
                        cameraTransform.position.x += gsl::narrow_cast<float>(translationPerSecond * mTime.delta);
                    }
                    if (mInput.keyDown(Key::W)) {
                        cameraTransform.position.y += gsl::narrow_cast<float>(translationPerSecond * mTime.delta);
                    }
                    if (mInput.keyDown(Key::S)) {
                        cameraTransform.position.y -= gsl::narrow_cast<float>(translationPerSecond * mTime.delta);
                    }
                    if (mInput.keyDown(Key::E)) {
                        cameraTransform.rotation -= gsl::narrow_cast<float>(rotationRadiansPerSecond * mTime.delta);
                    }
                    if (mInput.keyDown(Key::Q)) {
                        cameraTransform.rotation += gsl::narrow_cast<float>(rotationRadiansPerSecond * mTime.delta);
                    }
                    if (mInput.keyDown(Key::NumpadAdd)) {
                        cameraTransform.scale /=
                                gsl::narrow_cast<float>((zoomFactorPerSecond - 1.0) * mTime.delta + 1.0);
                    }
                    if (mInput.keyDown(Key::NumpadSubtract)) {
                        cameraTransform.scale *=
                                gsl::narrow_cast<float>((zoomFactorPerSecond - 1.0) * mTime.delta + 1.0);
                    }
                });
        mRegistry.emplaceSystem<>(
                []() {},
                [this, anchor]() {
                    /* this system sets the position of the first sprite to the current
                     * position of the mouse every frame */
                    auto& cameraTransform = std::get<TransformComponent&>(
                            mRegistry.componentsMutable<CameraComponent, TransformComponent>().front());

                    auto& transform = mRegistry.componentMutable<TransformComponent>(anchor).value();
                    const auto mousePosition = mInput.mousePosition();
                    const auto worldPosition = CameraComponent::screenToWorldPoint(mousePosition, cameraTransform);
                    transform.position.x = worldPosition.x;
                    transform.position.y = worldPosition.y;
                });
        mRegistry.emplaceSystem<const ScriptComponent&>([]() {},
                                                        [](Entity entity, const ScriptComponent& scriptComponent) {
                                                            scriptComponent.script->invokeUpdate(entity);
                                                        },
                                                        []() {});
        mRegistry.addScreenClearer(mAppContext, true, true);
        mRegistry.addDynamicSpriteRenderer(mAppContext, cameraEntity);
        mRegistry.addDynamicSpriteRenderer(mAppContext);// overlay
    }

    void Sandbox::update() noexcept { }

}// namespace c2k