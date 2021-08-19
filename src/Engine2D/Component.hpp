//
// Created by coder2k on 07.08.2021.
//

#pragma once

#include "Texture.hpp"
#include "ShaderProgram.hpp"
#include "WindowSize.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Transform {
    glm::vec3 position{ 0.0f };
    float rotation{ 0.0f };
    glm::vec2 scale{ 1.0f };

    [[nodiscard]] glm::mat4 matrix() const noexcept {
        return glm::scale(
                glm::rotate(glm::translate(glm::mat4{ 1.0f }, position), rotation, glm::vec3{ 0.0f, 0.0f, 1.0f }),
                glm::vec3{ scale.x, scale.y, 1.0f });
    }
};

struct DynamicSprite {
    const Texture* texture;
    ShaderProgram* shader;
    Color color;
};

struct Camera {
    [[nodiscard]] static glm::mat4 viewMatrix(const Transform& cameraTransform) noexcept {
        return glm::inverse(cameraTransform.matrix());
    }

    [[nodiscard]] static glm::mat4 projectionMatrix(const WindowSize& framebufferSize) {
        return glm::ortho<float>(gsl::narrow_cast<float>(-framebufferSize.width / 2),
                                 gsl::narrow_cast<float>(framebufferSize.width / 2),
                                 gsl::narrow_cast<float>(-framebufferSize.height / 2),
                                 gsl::narrow_cast<float>(framebufferSize.height / 2), -2.0f, 2.0f);
    }

    [[nodiscard]] static glm::mat4 viewProjectionMatrix(const Transform& cameraTransform,
                                                        const WindowSize& framebufferSize) {
        return projectionMatrix(framebufferSize) * viewMatrix(cameraTransform);
    }

    [[nodiscard]] static glm::vec3 screenToWorldPoint(const glm::vec2& screenPoint, const Transform& cameraTransform) {
        const glm::vec4 screenPoint4{ screenPoint.x, screenPoint.y, 0.0f, 1.0f };
        const auto matrix = glm::inverse(viewMatrix(cameraTransform));
        const auto transformedPoint4 = matrix * screenPoint4;
        return glm::vec3{ transformedPoint4.x, transformedPoint4.y, transformedPoint4.z };
    }
};