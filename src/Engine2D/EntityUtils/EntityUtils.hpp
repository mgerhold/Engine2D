//
// Created by coder2k on 20.11.2021.
//

#pragma once

#include "Entity.hpp"
#include "Registry.hpp"
#include <glm/glm.hpp>

namespace c2k::EntityUtils {

    [[nodiscard]] glm::mat4 getGlobalTransform(Registry& registry, Entity entity) noexcept;

}
