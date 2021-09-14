//
// Created by coder2k on 06.09.2021.
//

#include "ScriptUtils.hpp"
#include "Component.hpp"
#include "Registry.hpp"
#include "Time.hpp"
#include "Input.hpp"
#include "AssetDatabase.hpp"
#include "Rect.hpp"
#include "Color.hpp"
#define MAGIC_ENUM_RANGE_MAX 348
#include <magic_enum.hpp>

static_assert(MAGIC_ENUM_RANGE_MAX >= static_cast<std::underlying_type_t<c2k::Key>>(c2k::Key::LastKey));
static_assert(MAGIC_ENUM_RANGE_MAX >=
              static_cast<std::underlying_type_t<c2k::MouseButton>>(c2k::MouseButton::LastButton));
// TODO: Add more asserts to handle gamepad/joystick buttons/axes

namespace c2k::ScriptUtils {

    namespace {

        template<typename... Rest>
        inline void createNamespace(auto& parent, const std::string& first, Rest... rest) noexcept {
            auto namespaceTable = parent[first];
            if (!namespaceTable.valid()) {
                namespaceTable.set(sol::create);
            }
            if constexpr (sizeof...(Rest) > 0) {
                createNamespace(namespaceTable, rest...);
            }
        }

        inline void defineMathDataTypes(sol::state& luaState) noexcept {
            luaState.new_usertype<glm::vec3>("Vec3", "x", &glm::vec3::x, "y", &glm::vec3::y, "z", &glm::vec3::z);
            luaState.new_usertype<glm::vec2>("Vec2", "x", &glm::vec2::x, "y", &glm::vec2::y);
            luaState.new_usertype<Rect>("Rect", "left", &Rect::left, "right", &Rect::right, "top", &Rect::top, "bottom",
                                        &Rect::bottom);
        }

        inline void defineBasicEngineDataTypes(sol::state& luaState) noexcept {
            luaState.new_usertype<LuaEntity>("Entity", "id", &LuaEntity::id);
            luaState.new_usertype<Color>("Color", "r", &Color::r, "g", &Color::g, "b", &Color::b, "a", &Color::a);
            luaState.new_usertype<Time>("Time", "elapsed", sol::readonly(&Time::elapsed), "delta",
                                        sol::readonly(&Time::delta));
            luaState.new_usertype<LuaTexture>("Texture", "guid", &LuaTexture::guid, "width", &LuaTexture::width,
                                              "height", &LuaTexture::height, "numChannels", &LuaTexture::numChannels);
            luaState.new_usertype<LuaShaderProgram>("ShaderProgram", "guid", &LuaShaderProgram::guid);
            luaState.new_usertype<SpriteSheet::Frame>("SpriteSheetFrame", "rect", &SpriteSheet::Frame::rect,
                                                      "sourceWidth", &SpriteSheet::Frame::sourceWidth, "sourceHeight",
                                                      &SpriteSheet::Frame::sourceHeight);
            luaState.new_usertype<SpriteSheet>(
                    "SpriteSheet", "frames",
                    sol::property([](SpriteSheet& spriteSheet) -> std::vector<SpriteSheet::Frame>& {
                        return spriteSheet.frames;
                    }),
                    "texture", sol::property([&](const SpriteSheet& spriteSheet) {
                        const auto& texture = *spriteSheet.texture;
                        return LuaTexture{ .guid{ texture.guid.string() },
                                           .width{ texture.width() },
                                           .height{ texture.height() },
                                           .numChannels{ texture.numChannels() } };
                    }));
        }

        inline void defineTimeGetter(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
            luaState["c2k"]["getTime"] = [&]() { return applicationContext.time; };
        }

        namespace ComponentTypes {
            inline void defineTransformType(sol::state& luaState) {
                luaState.new_usertype<TransformComponent>("Transform", "position", &TransformComponent::position,
                                                          "rotation", &TransformComponent::rotation, "scale",
                                                          &TransformComponent::scale);
            }

            inline void defineDynamicSpriteType(ApplicationContext& applicationContext, sol::state& luaState) {
                luaState.new_usertype<DynamicSpriteComponent>(
                        "DynamicSprite", "textureRect", &DynamicSpriteComponent::textureRect, "color",
                        &DynamicSpriteComponent::color, "texture",
                        sol::property(
                                [](const DynamicSpriteComponent& sprite) {
                                    return ScriptUtils::LuaTexture{ .guid{ sprite.texture->guid.string() },
                                                                    .width{ sprite.texture->width() },
                                                                    .height{ sprite.texture->height() },
                                                                    .numChannels{ sprite.texture->numChannels() } };
                                },
                                [&](DynamicSpriteComponent& sprite, const LuaTexture& luaTexture) {
                                    sprite.texture = &applicationContext.assetDatabase.texture(
                                            GUID::fromString(luaTexture.guid));
                                }),
                        "shaderProgram",
                        sol::property(
                                [](const DynamicSpriteComponent& sprite) {
                                    return LuaShaderProgram{ .guid{ sprite.shaderProgram->guid.string() } };
                                },
                                [&](DynamicSpriteComponent& sprite, const LuaShaderProgram& luaShaderProgram) {
                                    sprite.shaderProgram = &applicationContext.assetDatabase.shaderProgramMutable(
                                            GUID::fromString(luaShaderProgram.guid));
                                }));
            }
        }// namespace ComponentTypes

        inline void defineComponentTypes(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
            ComponentTypes::defineTransformType(luaState);
            ComponentTypes::defineDynamicSpriteType(applicationContext, luaState);
        }

        namespace EntityAPI {

            inline void provideConstructionAPI(ApplicationContext& applicationContext,
                                               sol::usertype<LuaEntity>& entityType) noexcept {
                entityType["new"] = [&]() { return LuaEntity{ applicationContext.registry.createEntity() }; };
            }

            inline void provideDestructionAPI(ApplicationContext& applicationContext,
                                              sol::usertype<LuaEntity>& entityType) noexcept {
                entityType["destroy"] = [&](LuaEntity luaEntity) {
                    applicationContext.registry.destroyEntity(luaEntity);
                };
            }

            inline void provideTransformAPI(ApplicationContext& applicationContext,
                                            sol::usertype<LuaEntity>& entityType) noexcept {
                entityType["getTransform"] = [&](LuaEntity luaEntity) {
                    auto result = applicationContext.registry.componentMutable<TransformComponent>(luaEntity);
                    return result ? &result.value() : nullptr;
                };
                entityType["attachTransform"] = [&](LuaEntity luaEntity) {
                    applicationContext.registry.attachComponent<TransformComponent>(luaEntity, {});
                    return &applicationContext.registry.componentMutable<TransformComponent>(luaEntity).value();
                };
            }

            inline void provideDynamicSpriteAPI(ApplicationContext& applicationContext,
                                                sol::usertype<LuaEntity>& entityType) noexcept {
                entityType["getDynamicSprite"] = [&](LuaEntity luaEntity) {
                    auto result = applicationContext.registry.componentMutable<DynamicSpriteComponent>(luaEntity);
                    return result ? &result.value() : nullptr;
                };
                entityType["attachDynamicSprite"] = [&](LuaEntity luaEntity) {
                    const auto sprite = DynamicSpriteComponent{
                        .textureRect{ Rect::unit() },
                        .color{ Color::white() },
                        .texture{ &applicationContext.assetDatabase.fallbackTexture() },
                        .shaderProgram{ &applicationContext.assetDatabase.fallbackShaderProgramMutable() }
                    };
                    applicationContext.registry.attachComponent<DynamicSpriteComponent>(luaEntity, sprite);
                    return &applicationContext.registry.componentMutable<DynamicSpriteComponent>(luaEntity).value();
                };
            }

            inline void provideHierarchyAPI(ApplicationContext& applicationContext,
                                            sol::usertype<LuaEntity>& entityType) noexcept {
                entityType["hasRoot"] = [&](LuaEntity luaEntity) {
                    return applicationContext.registry.hasComponent<RootComponent>(luaEntity);
                };
                entityType["attachRoot"] = [&](LuaEntity luaEntity) {
                    applicationContext.registry.attachComponent<RootComponent>(luaEntity, RootComponent{});
                };
            }

        }// namespace EntityAPI

        inline void provideEntityAPI(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
            sol::usertype<LuaEntity> entityType = luaState["Entity"];
            EntityAPI::provideConstructionAPI(applicationContext, entityType);
            EntityAPI::provideDestructionAPI(applicationContext, entityType);
            EntityAPI::provideTransformAPI(applicationContext, entityType);
            EntityAPI::provideDynamicSpriteAPI(applicationContext, entityType);
            EntityAPI::provideHierarchyAPI(applicationContext, entityType);
        }

        namespace InputAPI {

            namespace Constants {

                inline void provideKeyConstants(sol::state& luaState) noexcept {
                    createNamespace(luaState, "Key");
                    for (const auto& entry : magic_enum::enum_entries<Key>()) {
                        luaState["Key"][entry.second] =
                                static_cast<typename std::underlying_type<Key>::type>(entry.first);
                    }
                }

                inline void provideMouseButtonConstants(sol::state& luaState) noexcept {
                    createNamespace(luaState, "MouseButton");
                    for (const auto& entry : magic_enum::enum_entries<MouseButton>()) {
                        luaState["MouseButton"][entry.second] =
                                static_cast<typename std::underlying_type<MouseButton>::type>(entry.first);
                    }
                }

            }// namespace Constants

            inline void defineInputType(sol::state& luaState) noexcept {
                luaState.new_usertype<Input>("Input", "keyDown", &Input::keyDown, "keyRepeated", &Input::keyRepeated,
                                             "keyPressed", &Input::keyPressed, "keyReleased", &Input::keyReleased,
                                             "mousePosition", &Input::mousePosition, "mouseInsideWindow",
                                             &Input::mouseInsideWindow, "mouseDown", &Input::mouseDown, "mousePressed",
                                             &Input::mousePressed, "mouseReleased", &Input::mouseReleased);
            }

            inline void provideInputGetter(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
                luaState["c2k"]["getInput"] = [&]() { return applicationContext.input; };
            }

        }// namespace InputAPI

        inline void provideInputAPI(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
            InputAPI::Constants::provideKeyConstants(luaState);
            InputAPI::Constants::provideMouseButtonConstants(luaState);
            InputAPI::defineInputType(luaState);
            InputAPI::provideInputGetter(applicationContext, luaState);
        }

        namespace AssetsAPI {
            inline void provideTextureAssetsAPI(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
                luaState["c2k"]["assets"]["texture"] = [&](const std::string& guidString) {
                    const auto& texture = applicationContext.assetDatabase.texture(GUID::fromString(guidString));
                    return LuaTexture{ .guid{ texture.guid.string() },
                                       .width{ texture.width() },
                                       .height{ texture.height() },
                                       .numChannels{ texture.numChannels() } };
                };
            }

            inline void provideShaderProgramAPI(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
                luaState["c2k"]["assets"]["shaderProgram"] = [&](const std::string& guidString) {
                    const auto& shaderProgram =
                            applicationContext.assetDatabase.shaderProgram(GUID::fromString(guidString));
                    return LuaShaderProgram{ .guid{ shaderProgram.guid.string() } };
                };
            }

            inline void provideSpriteSheetAPI(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
                luaState["c2k"]["assets"]["spriteSheet"] = [&](const std::string& guidString) {
                    const auto& spriteSheet =
                            applicationContext.assetDatabase.spriteSheet(GUID::fromString(guidString));
                    return spriteSheet;
                };
            }
        }// namespace AssetsAPI

        inline void provideAssetsAPI(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
            createNamespace(luaState, "c2k", "assets");
            AssetsAPI::provideTextureAssetsAPI(applicationContext, luaState);
            AssetsAPI::provideShaderProgramAPI(applicationContext, luaState);
            AssetsAPI::provideSpriteSheetAPI(applicationContext, luaState);
        }

        inline void defineTypes(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
            defineMathDataTypes(luaState);
            defineBasicEngineDataTypes(luaState);
            defineComponentTypes(applicationContext, luaState);
        }

        inline void provideAPIs(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
            provideInputAPI(applicationContext, luaState);
            defineTimeGetter(applicationContext, luaState);
            provideEntityAPI(applicationContext, luaState);
            provideAssetsAPI(applicationContext, luaState);
        }

    }// namespace

    void provideAPI(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
        createNamespace(luaState, "c2k");
        defineTypes(applicationContext, luaState);
        provideAPIs(applicationContext, luaState);
    }

}// namespace c2k::ScriptUtils