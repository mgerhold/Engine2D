//
// Created by coder2k on 07.08.2021.
//

#pragma once

#include "Texture.hpp"
#include "ShaderProgram.hpp"
#include <glm/glm.hpp>

struct Transform {
    glm::vec3 position;
    float rotation;
    glm::vec2 scale;
};

struct DynamicSprite {
    const Texture* texture;
    const ShaderProgram* shader;
    glm::vec3 color;
};