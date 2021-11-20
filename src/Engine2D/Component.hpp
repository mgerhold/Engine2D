//
// Created by coder2k on 07.08.2021.
//

#pragma once

#include "Texture.hpp"
#include "ShaderProgram.hpp"
#include "WindowSize.hpp"
#include "SpriteSheet.hpp"
#include "Rect.hpp"
#include "Entity.hpp"
#include "Script.hpp"
#include "ScriptUtils/ScriptUtils.hpp"
#include "ParticleSystem.hpp"
#include "Sprite.hpp"
#include "Animation.hpp"
#pragma warning(push)
#pragma warning(disable: 4201)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#pragma warning(pop)

namespace c2k {

    struct RootComponent { };

    struct RelationshipComponent {
        Entity parent;
    };

    struct TransformComponent {
        glm::vec3 position{ 0.0f };
        float rotation{ 0.0f };
        glm::vec2 scale{ 1.0f };

        [[nodiscard]] glm::mat4 matrix() const noexcept {
            return glm::scale(
                    glm::rotate(glm::translate(glm::mat4{ 1.0f }, position), rotation, glm::vec3{ 0.0f, 0.0f, 1.0f }),
                    glm::vec3{ scale.x, scale.y, 1.0f });
        }
        [[nodiscard]] static const TransformComponent& identity() noexcept {
            static auto result{ TransformComponent{} };
            return result;
        }
        [[nodiscard]] static TransformComponent fromMatrix(const glm::mat4& matrix) noexcept {
            glm::vec3 scale, translation, skew;
            glm::vec4 perspective;
            glm::quat orientation;
            glm::decompose(matrix, scale, orientation, translation, skew, perspective);
            auto eulerAngles = glm::eulerAngles(orientation);
            return TransformComponent{
                .position{ translation },
                .rotation{ eulerAngles.z },
                .scale{ glm::vec2{ scale.x, scale.y } }
            };
        }
    };

    struct DynamicSpriteComponent {
        ShaderProgram* shaderProgram;
        Sprite sprite;
        Color color;
    };

    struct SpriteSheetAnimationComponent {
        const SpriteSheet* spriteSheet;
        const Animation* animation;
        double lastFrameChange;
        int currentFrame{ 0 };
    };

    struct CameraComponent {
        [[nodiscard]] static glm::mat4 viewMatrix(const TransformComponent& cameraTransform) noexcept {
            return glm::inverse(cameraTransform.matrix());
        }

        [[nodiscard]] static glm::mat4 projectionMatrix(const WindowSize& framebufferSize) {
            return glm::ortho<float>(gsl::narrow_cast<float>(-framebufferSize.width / 2),
                                     gsl::narrow_cast<float>(framebufferSize.width / 2),
                                     gsl::narrow_cast<float>(-framebufferSize.height / 2),
                                     gsl::narrow_cast<float>(framebufferSize.height / 2), -2.0f, 2.0f);
        }

        [[nodiscard]] static glm::mat4 viewProjectionMatrix(const TransformComponent& cameraTransform,
                                                            const WindowSize& framebufferSize) {
            return projectionMatrix(framebufferSize) * viewMatrix(cameraTransform);
        }

        [[nodiscard]] static glm::vec3 screenToWorldPoint(const glm::vec2& screenPoint,
                                                          const TransformComponent& cameraTransform) {
            const glm::vec4 screenPoint4{ screenPoint.x, screenPoint.y, 0.0f, 1.0f };
            const auto matrix = glm::inverse(viewMatrix(cameraTransform));
            const auto transformedPoint4 = matrix * screenPoint4;
            return glm::vec3{ transformedPoint4.x, transformedPoint4.y, transformedPoint4.z };
        }
    };

    struct ScriptComponent {
        Script* script;
    };

    struct ParticleEmitterComponent {
        ParticleSystem particleSystem;
        double lastSpawnTime{ 0.0 };
    };

    struct ParticleComponent {
        double remainingLifeTime{ 0.0 };
        double totalLifeTime{ 0.0 };
        glm::vec3 velocity{ 0.0f };
        glm::vec3 gravity{ 0.0f };
        glm::vec2 startScale{ 1.0f };
        glm::vec2 endScale{ 1.0f };
        float startRotationSpeed{ 0.0f };
        float endRotationSpeed{ 0.0f };
    };

}// namespace c2k