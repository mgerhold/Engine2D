//
// Created by coder2k on 15.05.2021.
//

#include "Sandbox.hpp"
#include "AssetDatabase.hpp"
#include "Color.hpp"
#include "Component.hpp"
#include "GUID.hpp"
#include "Random.hpp"
#include "Registry.hpp"
#include "Sprite.hpp"
#include "SpriteSheet.hpp"
#include "Texture.hpp"
#include "Time.hpp"

#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include <filesystem>
#include <range/v3/all.hpp>

#include <memory>
#include <new>
#include <utility>
#include <vector>

namespace c2k {

    void Sandbox::setup() noexcept {
        mAssetDatabase.loadFromList(AssetDatabase::assetPath() / "scenes" / "assets.json");

        const auto spriteSheetGUID{ GUID::fromString("c15111ea-7ba8-4e65-8f24-40c868498d5b") };
        const auto fireTextureGUID{ GUID::fromString("c22764c9-9750-4749-810e-10f4c6f50123") };
        const auto textureGUID{ GUID::fromString("9043b452-363c-4917-bfde-592a72077e37") };
        const auto shaderGUID{ GUID::fromString("b520f0eb-1756-41e0-ac07-66c3338bc594") };
        const auto fireAnimationGUID{ GUID::fromString("11d93892-4542-4177-9c83-00647858fbe3") };

        mAssetDatabase.textureMutable(fireTextureGUID).setFiltering(Texture::Filtering::Nearest);

        // generate game scene
        constexpr float textureHeight = 40.0f;
        const glm::vec2 textureSize{ textureHeight * mAssetDatabase.texture(textureGUID).widthToHeightRatio(),
                                     textureHeight };
        const glm::vec2 fireTextureSize{
            textureHeight * mAssetDatabase.spriteSheet(spriteSheetGUID).frames[0].getWidthToHeightRatio(), textureHeight
        };
        mRegistry.createEntity(TransformComponent{ .position{ 0.0f, 0.0f, 0.7f } }, RootComponent{},
                               ParticleEmitterComponent{ .particleSystem{ mAssetDatabase.particleSystem(
                                       GUID::fromString("bd59709a-cc71-4bed-bb46-dc7624686ef6")) } });
        /*mRegistry.createEntity(TransformComponent{ .position{ 100.0f, 0.0f, 0.7f } }, RootComponent{},
                               ParticleEmitterComponent{ .particleSystem{ &mAssetDatabase.particleSystem(
                                       GUID::fromString("cb15abcf-7a3a-4463-8ee2-93e8435cef63")) } });*/
        // entity with spritesheettest.lua script attached
        mRegistry.createEntity(ScriptComponent{
                .script{ &mAssetDatabase.scriptMutable(GUID::fromString("afcdc904-20a0-4185-8d8f-d9ba0d61eb37")) } });
        // entity with entityspawner.lua script attached
        mRegistry.createEntity(ScriptComponent{
                .script{ &mAssetDatabase.scriptMutable(GUID::fromString("9632ed81-76ce-469c-b909-09a754877ae9")) } });
        const auto anchor =
                mRegistry.createEntity(TransformComponent{ .position{ 0.0f, 180.0f, 0.9f } }, RootComponent{});
        mRegistry.createEntity(
                TransformComponent{ .scale{ fireTextureSize * 2.0f } },
                DynamicSpriteComponent{
                        .shaderProgram{ &mAssetDatabase.shaderProgramMutable(shaderGUID) },
                        .sprite{ Sprite::fromSpriteSheet(mAssetDatabase.spriteSheet(spriteSheetGUID), 0) },
                        .color{ Color::white() },
                },
                SpriteSheetAnimationComponent{ .spriteSheet{ &mAssetDatabase.spriteSheet(spriteSheetGUID) },
                                               .animation{ &mAssetDatabase.animation(fireAnimationGUID) },
                                               .lastFrameChange{ mTime.elapsed } },
                RelationshipComponent{ .parent{ anchor } });
        const auto secondFlame = mRegistry.createEntity(
                TransformComponent{ .position{ 200.0f, 0.0f, 0.0f }, .scale{ fireTextureSize } },
                DynamicSpriteComponent{
                        .shaderProgram{ &mAssetDatabase.shaderProgramMutable(shaderGUID) },
                        .sprite{ Sprite::fromSpriteSheet(mAssetDatabase.spriteSheet(spriteSheetGUID), 0) },
                        .color{ Color::white() },
                },
                SpriteSheetAnimationComponent{ .spriteSheet{ &mAssetDatabase.spriteSheet(spriteSheetGUID) },
                                               .animation{ &mAssetDatabase.animation(fireAnimationGUID) },
                                               .lastFrameChange{ mTime.elapsed },
                                               .currentFrame{ 0 } },
                RelationshipComponent{ .parent{ anchor } });
        mRegistry.createEntity(
                TransformComponent{ .position{ 5.0f, 0.0f, 0.0f }, .scale{ 0.5f, 0.5f } },
                DynamicSpriteComponent{
                        .shaderProgram{ &mAssetDatabase.shaderProgramMutable(shaderGUID) },
                        .sprite{ Sprite::fromSpriteSheet(mAssetDatabase.spriteSheet(spriteSheetGUID), 0) },
                        .color{ Color::white() },
                },
                SpriteSheetAnimationComponent{ .spriteSheet{ &mAssetDatabase.spriteSheet(spriteSheetGUID) },
                                               .animation{ &mAssetDatabase.animation(fireAnimationGUID) },
                                               .lastFrameChange{ mTime.elapsed },
                                               .currentFrame{ 0 } },
                RelationshipComponent{ .parent{ secondFlame } });

        constexpr int numEntities = 100;
        for ([[maybe_unused]] auto _ : ranges::views::ints(0, numEntities)) {
            const glm::vec3 position{ mRandom.range(-2000.0f, 2000.0f), mRandom.range(-2000.0f, 2000.0f), 0.8f };
            mRegistry.createEntity(TransformComponent{ .position{ position }, .scale{ textureSize * 2.0f } },
                                   DynamicSpriteComponent{
                                           .shaderProgram{ &mAssetDatabase.shaderProgramMutable(shaderGUID) },
                                           .sprite{ Sprite::fromTexture(mAssetDatabase.texture(textureGUID)) },
                                           .color{ Color::white() },
                                   },
                                   RootComponent{},
                                   ScriptComponent{ .script = &mAssetDatabase.scriptMutable(GUID::fromString(
                                                            "5874d6e9-5529-45bc-829b-d6002ef21d70")) });
        }
    }

    void Sandbox::update() noexcept { }

}// namespace c2k