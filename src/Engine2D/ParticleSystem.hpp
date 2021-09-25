//
// Created by coder2k on 25.09.2021.
//

#pragma once

#include "Texture.hpp"
#include <filesystem>

namespace c2k {

    class ShaderProgram;

    struct ParticleSystem {
        const Texture* texture{ nullptr };
        ShaderProgram* shaderProgram{ nullptr };
        float startLifeTime{ 0.0f };
        float lifeTimeVariation{ 0.0f };
        float particlesPerSecond{ 0.0f };
        glm::vec3 gravity{ 0.0f };
        glm::vec2 startScale{ 1.0f };
        glm::vec2 endScale{ 1.0f };
        float startRotationSpeed{ 0.0f };
        float endRotationSpeed{ 0.0f };
        float startRotationSpeedVariation{ 0.0f };
        float endRotationSpeedVariation{ 0.0f };

        static tl::expected<ParticleSystem, std::string> loadFromFile(const std::filesystem::path& filename,
                                                                      const Texture& texture,
                                                                      ShaderProgram& shaderProgram) noexcept;
    };

}// namespace c2k