//
// Created by coder2k on 07.08.2021.
//

#pragma once

#include "Texture.hpp"
#include "ShaderProgram.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Transform {
    glm::vec3 position;
    float rotation;
    glm::vec2 scale;

    [[nodiscard]] glm::mat4 matrix() const noexcept {
        return glm::scale(
                glm::rotate(glm::translate(glm::mat4{ 1.0f }, position), rotation, glm::vec3{ 0.0f, 0.0f, 1.0f }),
                glm::vec3{ scale.x, scale.y, 1.0f });
    }
};

struct DynamicSprite {
    const Texture* texture;
    ShaderProgram* shader;
    glm::vec3 color;
};

struct Camera {
    [[nodiscard]] static glm::mat4 matrix(const Transform& transform) noexcept {
        return glm::inverse(transform.matrix());
    }
};