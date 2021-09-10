//
// Created by coder2k on 06.09.2021.
//

#include "ScriptUtils.hpp"
#include "Component.hpp"
#include "Entity.hpp"
#include "Registry.hpp"
#include "Time.hpp"
#include "Input.hpp"
#define MAGIC_ENUM_RANGE_MAX 348
#include <magic_enum.hpp>

static_assert(MAGIC_ENUM_RANGE_MAX >= static_cast<std::underlying_type_t<c2k::Key>>(c2k::Key::LastKey));
static_assert(MAGIC_ENUM_RANGE_MAX >=
              static_cast<std::underlying_type_t<c2k::MouseButton>>(c2k::MouseButton::LastButton));
// TODO: Add more asserts to handle gamepad/joystick buttons/axes

namespace c2k::ScriptUtils {

    void registerTypes(sol::state& luaState) noexcept {
        luaState.new_usertype<glm::vec3>("Vec3", "x", &glm::vec3::x, "y", &glm::vec3::y, "z", &glm::vec3::z);
        luaState.new_usertype<glm::vec2>("Vec2", "x", &glm::vec2::x, "y", &glm::vec2::y);
        luaState.new_usertype<TransformComponent>("Transform", "position", &TransformComponent::position, "rotation",
                                                  &TransformComponent::rotation, "scale", &TransformComponent::scale);
        luaState.new_usertype<Time>("Time", "elapsed", sol::readonly(&Time::elapsed), "delta",
                                    sol::readonly(&Time::delta));
        luaState["Key"] = luaState.create_table();
        for (const auto& entry : magic_enum::enum_entries<Key>()) {
            luaState["Key"][entry.second] = static_cast<typename std::underlying_type<Key>::type>(entry.first);
        }
        luaState["MouseButton"] = luaState.create_table();
        for (const auto& entry : magic_enum::enum_entries<MouseButton>()) {
            luaState["MouseButton"][entry.second] =
                    static_cast<typename std::underlying_type<MouseButton>::type>(entry.first);
        }
        luaState.new_usertype<Input>("Input", "keyDown", &Input::keyDown, "keyRepeated", &Input::keyRepeated,
                                     "keyReleased", &Input::keyReleased, "mousePosition", &Input::mousePosition,
                                     "mouseInsideWindow", &Input::mouseInsideWindow, "mouseDown", &Input::mouseDown,
                                     "mousePressed", &Input::mousePressed, "mouseReleased", &Input::mouseReleased);
    }

    void provideAPI(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
        luaState["getTransform"] = [&](Entity entity) {
            auto result = applicationContext.registry.componentMutable<TransformComponent>(entity);
            return result ? &result.value() : nullptr;
        };
        luaState["getTime"] = [&]() { return applicationContext.time; };
        luaState["getInput"] = [&]() { return applicationContext.input; };
    }

}// namespace c2k::ScriptUtils