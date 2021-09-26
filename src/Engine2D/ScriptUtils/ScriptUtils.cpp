//
// Created by coder2k on 06.09.2021.
//

#include "ScriptUtils.hpp"
#include "../Component.hpp"
#include "../Registry.hpp"
#include "../Time.hpp"
#include "../Input.hpp"
#include "../AssetDatabase.hpp"
#include "../Rect.hpp"
#include "../Color.hpp"
#include "../Application.hpp"
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
            luaState.new_usertype<LuaScript>("Script", "guid", &LuaScript::guid);
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
            luaState["c2k"]["getTime"] = [&]() {
                // return as pointer to allow reference semantics in Lua
                return &applicationContext.time;
            };
        }

        namespace ComponentTypes {
            inline void defineTransformType(ApplicationContext& applicationContext, sol::state& luaState) {
                luaState.new_usertype<LuaTransform>(
                        "Transform", "position", sol::property([&](const LuaTransform& luaTransform) -> glm::vec3* {
                            auto&& transform = applicationContext.registry.componentMutable<TransformComponent>(
                                    luaTransform.owningEntity);
                            if (!transform) {
                                spdlog::error("Invalid transform handle.");
                                return nullptr;
                            }
                            return &(transform->position);
                        }),
                        "rotation",
                        sol::property(
                                [&](const LuaTransform& luaTransform) {
                                    auto&& transform = applicationContext.registry.component<TransformComponent>(
                                            luaTransform.owningEntity);
                                    if (!transform) {
                                        spdlog::error("Invalid transform handle.");
                                        return 0.0f;
                                    }
                                    return transform->rotation;
                                },
                                [&](const LuaTransform& luaTransform, float rotation) {
                                    auto&& transform = applicationContext.registry.componentMutable<TransformComponent>(
                                            luaTransform.owningEntity);
                                    if (!transform) {
                                        spdlog::error("Invalid transform handle.");
                                        return;
                                    }
                                    transform->rotation = rotation;
                                }),
                        "scale", sol::property([&](const LuaTransform& luaTransform) -> glm::vec2* {
                            auto&& transform = applicationContext.registry.componentMutable<TransformComponent>(
                                    luaTransform.owningEntity);
                            if (!transform) {
                                spdlog::error("Invalid transform handle.");
                                return nullptr;
                            }
                            return &(transform->scale);
                        }));
            }

            inline void defineDynamicSpriteType(ApplicationContext& applicationContext, sol::state& luaState) {
                luaState.new_usertype<LuaDynamicSprite>(
                        "DynamicSprite", "textureRect",
                        sol::property(
                                [&](const LuaDynamicSprite& luaSprite) {
                                    auto&& dynamicSprite{ applicationContext.registry.component<DynamicSpriteComponent>(
                                            luaSprite.owningEntity) };
                                    if (!dynamicSprite) {
                                        spdlog::error("Invalid dynamic sprite component");
                                        return Rect{};
                                    }
                                    return dynamicSprite.value().sprite.textureRect;
                                },
                                [&](const LuaDynamicSprite& luaSprite, const Rect& textureRect) {
                                    auto&& dynamicSprite{
                                        applicationContext.registry.componentMutable<DynamicSpriteComponent>(
                                                luaSprite.owningEntity)
                                    };
                                    if (!dynamicSprite) {
                                        spdlog::error("Invalid dynamic sprite component");
                                        return;
                                    }
                                    dynamicSprite.value().sprite.textureRect = textureRect;
                                }),
                        "color",
                        sol::property(
                                [&](const LuaDynamicSprite& luaSprite) {
                                    auto&& dynamicSprite{ applicationContext.registry.component<DynamicSpriteComponent>(
                                            luaSprite.owningEntity) };
                                    if (!dynamicSprite) {
                                        spdlog::error("Invalid dynamic sprite component");
                                        return Color{};
                                    }
                                    return dynamicSprite.value().color;
                                },
                                [&](const LuaDynamicSprite& luaSprite, const Color& color) {
                                    auto&& dynamicSprite{
                                        applicationContext.registry.componentMutable<DynamicSpriteComponent>(
                                                luaSprite.owningEntity)
                                    };
                                    if (!dynamicSprite) {
                                        spdlog::error("Invalid dynamic sprite component");
                                        return;
                                    }
                                    dynamicSprite.value().color = color;
                                }),
                        "texture",
                        sol::property(
                                [&](const LuaDynamicSprite& luaSprite) {
                                    auto&& dynamicSprite{ applicationContext.registry.component<DynamicSpriteComponent>(
                                            luaSprite.owningEntity) };
                                    if (!dynamicSprite) {
                                        spdlog::error("Invalid dynamic sprite component");
                                        return LuaTexture{};
                                    }
                                    return LuaTexture{ .guid{ dynamicSprite->sprite.texture->guid.string() },
                                                       .width{ dynamicSprite->sprite.texture->width() },
                                                       .height{ dynamicSprite->sprite.texture->height() },
                                                       .numChannels{ dynamicSprite->sprite.texture->numChannels() } };
                                },
                                [&](const LuaDynamicSprite& luaSprite, const LuaTexture& luaTexture) {
                                    auto&& dynamicSprite{
                                        applicationContext.registry.componentMutable<DynamicSpriteComponent>(
                                                luaSprite.owningEntity)
                                    };
                                    if (!dynamicSprite) {
                                        spdlog::error("Invalid dynamic sprite component");
                                        return;
                                    }
                                    dynamicSprite->sprite.texture = &applicationContext.assetDatabase.texture(
                                            GUID::fromString(luaTexture.guid));
                                }),
                        "shaderProgram",
                        sol::property(
                                [&](const LuaDynamicSprite& luaSprite) {
                                    auto&& dynamicSprite{ applicationContext.registry.component<DynamicSpriteComponent>(
                                            luaSprite.owningEntity) };
                                    if (!dynamicSprite) {
                                        spdlog::error("Invalid dynamic sprite component");
                                        return LuaShaderProgram{};
                                    }
                                    return LuaShaderProgram{ .guid{ dynamicSprite->shaderProgram->guid.string() } };
                                },
                                [&](const LuaDynamicSprite& luaSprite, const LuaShaderProgram& luaShaderProgram) {
                                    auto&& dynamicSprite{
                                        applicationContext.registry.componentMutable<DynamicSpriteComponent>(
                                                luaSprite.owningEntity)
                                    };
                                    if (!dynamicSprite) {
                                        spdlog::error("Invalid dynamic sprite component");
                                        return;
                                    }
                                    dynamicSprite->shaderProgram =
                                            &applicationContext.assetDatabase.shaderProgramMutable(
                                                    GUID::fromString(luaShaderProgram.guid));
                                }));
            }
        }// namespace ComponentTypes

        inline void defineComponentTypes(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
            ComponentTypes::defineTransformType(applicationContext, luaState);
            ComponentTypes::defineDynamicSpriteType(applicationContext, luaState);
        }

        namespace EntityAPI {

            inline void provideConstructionAPI(ApplicationContext& applicationContext,
                                               sol::usertype<LuaEntity>& entityType) noexcept {
                entityType["new"] = [&]() { return LuaEntity{ applicationContext.registry.createEntity() }; };
                entityType["invalid"] = []() { return LuaEntity{ invalidEntity }; };
            }

            inline void provideDestructionAPI(ApplicationContext& applicationContext,
                                              sol::usertype<LuaEntity>& entityType) noexcept {
                entityType["destroy"] = [&](LuaEntity& luaEntity) {
                    applicationContext.bufferedScriptCommands.emplace_back(
                            BufferedScriptCommands::DestroyEntity{ .targetEntity{ luaEntity } });
                    luaEntity = invalidEntity;
                };
            }

            inline void provideTransformAPI(ApplicationContext& applicationContext,
                                            sol::usertype<LuaEntity>& entityType) noexcept {
                entityType["getTransform"] = [&](LuaEntity luaEntity) {
                    const Entity entity{ luaEntity };
                    const auto transform = applicationContext.registry.component<TransformComponent>(entity);
                    if (!transform) {
                        spdlog::error("Entity {} does not have a transform component.", entity);
                        return LuaTransform{ .owningEntity{ invalidEntity } };
                    }
                    return LuaTransform{ .owningEntity{ entity } };
                };
                entityType["attachTransform"] = [&](LuaEntity luaEntity) {
                    const Entity entity{ luaEntity };
                    if (applicationContext.registry.hasComponent<TransformComponent>(entity)) {
                        spdlog::error("Entity {} already has a transform component.", entity);
                    } else {
                        applicationContext.registry.attachComponent<TransformComponent>(entity, {});
                    }
                    return LuaTransform{ .owningEntity{ entity } };
                };
            }

            inline void provideScriptAPI(ApplicationContext& applicationContext,
                                         sol::usertype<LuaEntity>& entityType) noexcept {
                entityType["attachScript"] = [&](LuaEntity luaEntity, const LuaScript& luaScript) {
                    applicationContext.bufferedScriptCommands.emplace_back(
                            BufferedScriptCommands::AttachScript{ .scriptGUID{ GUID::fromString(luaScript.guid) },
                                                                  .targetEntity{ luaEntity } });
                };
            }

            inline void provideDynamicSpriteAPI(ApplicationContext& applicationContext,
                                                sol::usertype<LuaEntity>& entityType) noexcept {
                entityType["getDynamicSprite"] = [&](LuaEntity luaEntity) {
                    const Entity entity{ luaEntity };
                    const auto result = applicationContext.registry.componentMutable<DynamicSpriteComponent>(entity);
                    if (result) {
                        return LuaDynamicSprite{ .owningEntity{ entity } };
                    }
                    spdlog::error("Entity {} does not have a dynamic sprite component.", entity);
                    return LuaDynamicSprite{ .owningEntity{ invalidEntity } };
                };
                entityType["attachDynamicSprite"] = [&](LuaEntity luaEntity) {
                    const Entity entity{ luaEntity };
                    if (applicationContext.registry.hasComponent<DynamicSpriteComponent>(entity)) {
                        spdlog::error("Entity {} already has a dynamic sprite component.", entity);
                    } else {
                        const auto sprite = DynamicSpriteComponent{
                            .shaderProgram{ &applicationContext.assetDatabase.fallbackShaderProgramMutable() },
                            .sprite{ Sprite::fromTexture(applicationContext.assetDatabase.fallbackTexture()) },
                            .color{ Color::white() },
                        };
                        applicationContext.registry.attachComponent<DynamicSpriteComponent>(entity, sprite);
                    }
                    return LuaDynamicSprite{ .owningEntity{ entity } };
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
            EntityAPI::provideScriptAPI(applicationContext, entityType);
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
                luaState["c2k"]["getInput"] = [&]() {
                    // return as pointer to allow reference semantics in Lua
                    return &applicationContext.input;
                };
            }

        }// namespace InputAPI

        inline void provideInputAPI(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
            InputAPI::Constants::provideKeyConstants(luaState);
            InputAPI::Constants::provideMouseButtonConstants(luaState);
            InputAPI::defineInputType(luaState);
            InputAPI::provideInputGetter(applicationContext, luaState);
        }

        namespace AssetsAPI {
            inline void provideTextureAPI(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
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

            inline void provideScriptAPI(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
                luaState["c2k"]["assets"]["script"] = [&](const std::string& guidString) {
                    const auto& script = applicationContext.assetDatabase.scriptMutable(GUID::fromString(guidString));
                    return LuaScript{ .guid{ script.guid.string() } };
                };
            }
        }// namespace AssetsAPI

        inline void provideAssetsAPI(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
            createNamespace(luaState, "c2k", "assets");
            AssetsAPI::provideTextureAPI(applicationContext, luaState);
            AssetsAPI::provideShaderProgramAPI(applicationContext, luaState);
            AssetsAPI::provideSpriteSheetAPI(applicationContext, luaState);
            AssetsAPI::provideScriptAPI(applicationContext, luaState);
        }

        inline void provideApplicationAPI(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
            createNamespace(luaState, "c2k", "application");
            luaState["c2k"]["application"]["quit"] = [&]() { applicationContext.application.quit(); };
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
            provideApplicationAPI(applicationContext, luaState);
        }

    }// namespace

    void provideAPI(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
        createNamespace(luaState, "c2k");
        defineTypes(applicationContext, luaState);
        provideAPIs(applicationContext, luaState);
    }

}// namespace c2k::ScriptUtils