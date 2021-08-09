//
// Created by coder2k on 07.08.2021.
//

#pragma once

#include <glm/glm.hpp>

struct Transform {
    glm::vec3 position;
    float rotation;
    glm::vec2 scale;
};

struct DynamicSprite {
    // GUID texture;
    glm::vec3 color;
};