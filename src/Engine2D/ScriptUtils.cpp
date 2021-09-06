//
// Created by coder2k on 06.09.2021.
//

#include "ScriptUtils.hpp"
#include "Component.hpp"
#include "Entity.hpp"
#include "Registry.hpp"

namespace c2k::ScriptUtils {

    void registerTypes(sol::state& luaState) noexcept {
        luaState.new_usertype<glm::vec3>("Vec3", "x", &glm::vec3::x, "y", &glm::vec3::y, "z", &glm::vec3::z);
        luaState.new_usertype<glm::vec2>("Vec2", "x", &glm::vec2::x, "y", &glm::vec2::y);
        luaState.new_usertype<TransformComponent>("Transform", "position", &TransformComponent::position, "rotation",
                                                  &TransformComponent::rotation, "scale", &TransformComponent::scale);
    }

    void provideAPI(ApplicationContext& applicationContext, sol::state& luaState) noexcept {
        luaState["getTransform"] = [&](Entity entity) {
            auto result = applicationContext.registry.componentMutable<TransformComponent>(entity);
            return result ? &result.value() : nullptr;
        };
    }

}// namespace c2k::ScriptUtils