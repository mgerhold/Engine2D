//
// Created by coder2k on 20.11.2021.
//

#include "EntityUtils.hpp"
#include <cassert>

namespace c2k::EntityUtils {

    glm::mat4 getGlobalTransform(Registry& registry, const Entity entity) noexcept {
        if (!registry.hasComponent<TransformComponent>(entity)) {
            spdlog::error(
                    "Trying to get global transform of entity {} but this entity does not have a transform component.",
                    entity);
            return glm::mat4{ 1.0f };
        }
        const auto transform = registry.component<TransformComponent>(entity).value();
        auto transformMatrix = transform.matrix();
        Entity current = entity;
        while (registry.hasComponent<RelationshipComponent>(current)) {
            current = registry.component<RelationshipComponent>(current)->parent;
            assert(registry.isEntityAlive(current));
            const auto parentTransform = (registry.hasComponent<TransformComponent>(current)
                                                  ? registry.component<TransformComponent>(current).value().matrix()
                                                  : glm::mat4{ 1.0f });
            transformMatrix = parentTransform * transformMatrix;
        }
        return transformMatrix;
    }

}// namespace c2k::EntityUtils
