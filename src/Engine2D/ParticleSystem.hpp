//
// Created by coder2k on 25.09.2021.
//

#pragma once

#include "Sprite.hpp"
#include "JSON/JSON.hpp"
#include "Color.hpp"
#include "GUID.hpp"
#include <glm/glm.hpp>
#include <filesystem>

namespace c2k {

#include "ParticleSystemJSONTypes.inc"

    class ShaderProgram;

    struct ParticleSystem : public ParticleSystemImpl::ParticleSystemJSON {
    public:
        Sprite sprite;
        ShaderProgram* shaderProgram{ nullptr };
        GUID guid;

    public:
        ParticleSystem& operator<<(const ParticleSystemImpl::ParticleSystemJSON& base) noexcept {
            ParticleSystemImpl::ParticleSystemJSON::operator=(base);
            return *this;
        }

        static tl::expected<ParticleSystem, std::string> loadFromFile(const std::filesystem::path& filename,
                                                                      const Texture& texture,
                                                                      ShaderProgram& shaderProgram,
                                                                      GUID guid = GUID{}) noexcept;
    };

}// namespace c2k