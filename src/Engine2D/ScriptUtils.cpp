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

    inline void provideMathDataTypes(sol::state& luaState) noexcept {
        luaState.new_usertype<glm::vec3>("Vec3", "x", &glm::vec3::x, "y", &glm::vec3::y, "z", &glm::vec3::z);
        luaState.new_usertype<glm::vec2>("Vec2", "x", &glm::vec2::x, "y", &glm::vec2::y);
        luaState.new_usertype<Rect>("Rect", "left", &Rect::left, "right", &Rect::right, "top", &Rect::top, "bottom",
                                    &Rect::bottom);
    }

    inline void provideBasicEngineDataTypes(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
        luaState.new_usertype<LuaEntity>("Entity", "id", &LuaEntity::id);
        luaState.new_usertype<Color>("Color", "r", &Color::r, "g", &Color::g, "b", &Color::b, "a", &Color::a);
        luaState.new_usertype<Time>("Time", "elapsed", sol::readonly(&Time::elapsed), "delta",
                                    sol::readonly(&Time::delta));
        luaState.new_usertype<LuaTexture>("Texture", "guid", &LuaTexture::guid, "width", &LuaTexture::width, "height",
                                          &LuaTexture::height, "numChannels", &LuaTexture::numChannels);
        luaState.new_usertype<LuaShaderProgram>("ShaderProgram", "guid", &LuaShaderProgram::guid);
        luaState["c2k"]["getTime"] = [&]() { return applicationContext.time; };
    }

    inline void provideEntityAPI(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
        sol::usertype<LuaEntity> entityType = luaState["Entity"];
        entityType["new"] = [&]() { return LuaEntity{ applicationContext.registry.createEntity() }; };
        entityType["destroy"] = [&](LuaEntity luaEntity) { applicationContext.registry.destroyEntity(luaEntity); };
        entityType["getTransform"] = [&](LuaEntity luaEntity) {
            auto result = applicationContext.registry.componentMutable<TransformComponent>(luaEntity);
            return result ? &result.value() : nullptr;
        };
        entityType["attachTransform"] = [&](LuaEntity luaEntity) {
            applicationContext.registry.attachComponent<TransformComponent>(luaEntity, {});
            return &applicationContext.registry.componentMutable<TransformComponent>(luaEntity).value();
        };
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
        entityType["hasRoot"] = [&](LuaEntity luaEntity) {
            return applicationContext.registry.hasComponent<RootComponent>(luaEntity);
        };
        entityType["attachRoot"] = [&](LuaEntity luaEntity) {
            applicationContext.registry.attachComponent<RootComponent>(luaEntity, RootComponent{});
        };
    }

    inline void provideInputAPI(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
        createNamespace(luaState, "Key");
        for (const auto& entry : magic_enum::enum_entries<Key>()) {
            luaState["Key"][entry.second] = static_cast<typename std::underlying_type<Key>::type>(entry.first);
        }
        createNamespace(luaState, "MouseButton");
        for (const auto& entry : magic_enum::enum_entries<MouseButton>()) {
            luaState["MouseButton"][entry.second] =
                    static_cast<typename std::underlying_type<MouseButton>::type>(entry.first);
        }
        luaState.new_usertype<Input>("Input", "keyDown", &Input::keyDown, "keyRepeated", &Input::keyRepeated,
                                     "keyPressed", &Input::keyPressed, "keyReleased", &Input::keyReleased,
                                     "mousePosition", &Input::mousePosition, "mouseInsideWindow",
                                     &Input::mouseInsideWindow, "mouseDown", &Input::mouseDown, "mousePressed",
                                     &Input::mousePressed, "mouseReleased", &Input::mouseReleased);
        luaState["c2k"]["getInput"] = [&]() { return applicationContext.input; };
    }

    inline void provideComponentAPI(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
        luaState.new_usertype<TransformComponent>("Transform", "position", &TransformComponent::position, "rotation",
                                                  &TransformComponent::rotation, "scale", &TransformComponent::scale);
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
                            sprite.texture =
                                    &applicationContext.assetDatabase.texture(GUID::fromString(luaTexture.guid));
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

    inline void provideAssetsAPI(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
        createNamespace(luaState, "c2k", "assets");
        luaState["c2k"]["assets"]["texture"] = [&](const std::string& guidString) {
            const auto& texture = applicationContext.assetDatabase.texture(GUID::fromString(guidString));
            return LuaTexture{ .guid{ texture.guid.string() },
                               .width{ texture.width() },
                               .height{ texture.height() },
                               .numChannels{ texture.numChannels() } };
        };
        luaState["c2k"]["assets"]["shaderProgram"] = [&](const std::string& guidString) {
            const auto& shaderProgram = applicationContext.assetDatabase.shaderProgram(GUID::fromString(guidString));
            return LuaShaderProgram{ .guid{ shaderProgram.guid.string() } };
        };
    }

    void provideAPI(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
        createNamespace(luaState, "c2k");
        provideMathDataTypes(luaState);
        provideBasicEngineDataTypes(applicationContext, luaState);
        provideComponentAPI(applicationContext, luaState);
        provideInputAPI(applicationContext, luaState);
        provideEntityAPI(applicationContext, luaState);
        provideAssetsAPI(applicationContext, luaState);
    }

}// namespace c2k::ScriptUtils