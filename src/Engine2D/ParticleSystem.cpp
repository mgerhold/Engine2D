//
// Created by coder2k on 25.09.2021.
//

#include "ParticleSystem.hpp"
#include "FileUtils/FileUtils.hpp"
#include "JSONUtils.hpp"
#include "ShaderProgram.hpp"
#include <gsl/gsl>
#include <tl/expected.hpp>

namespace c2k {

    tl::expected<ParticleSystem, std::string> ParticleSystem::loadFromFile(const std::filesystem::path& filename,
                                                                           const Texture& texture,
                                                                           ShaderProgram& shaderProgram,
                                                                           GUID guid) noexcept {
        using namespace JSONUtils;
        const auto parseResult = JSON::fromFile(filename);
        if (!parseResult) {
            return tl::unexpected(fmt::format("Failed to parse particle system: {}", parseResult.error()));
        }
        const auto deserializationResult = parseResult->as(std::type_identity<ParticleSystemJSON>{});
        if (!deserializationResult) {
            return tl::unexpected(fmt::format("Unable to deserialize particle system"));
        }
        const auto particleSystemJSON = deserializationResult.value();
        ParticleSystem result;
        result << particleSystemJSON;// only assigns members that are inherited from ParticleSystemJSON
        // assign remaining members
        result.sprite = Sprite::fromTexture(texture);
        result.shaderProgram = &shaderProgram;
        result.guid = guid;
        return result;
    }
}// namespace c2k