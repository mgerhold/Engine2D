//
// Created by coder2k on 25.09.2021.
//

#include "ParticleSystem.hpp"
#include "FileUtils/FileUtils.hpp"
#include "JSONUtils.hpp"
#include "ShaderProgram.hpp"

namespace c2k {

    namespace {

        struct Vec2JSON {
            float x;
            float y;

            operator glm::vec2() const {
                return glm::vec2{ x, y };
            }
        };

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vec2JSON, x, y);

        struct Vec3JSON {
            float x;
            float y;
            float z;

            operator glm::vec3() const {
                return glm::vec3{ x, y, z };
            }
        };

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vec3JSON, x, y, z);

        struct ParticleSystemJSON {
            float startLifeTime;
            float lifeTimeVariation;
            float particlesPerSecond;
            Vec3JSON gravity;
            Vec2JSON startScale;
            Vec2JSON endScale;
            float startRotationSpeed;
            float endRotationSpeed;
            float startRotationSpeedVariation;
            float endRotationSpeedVariation;
        };

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ParticleSystemJSON,
                                           startLifeTime,
                                           lifeTimeVariation,
                                           particlesPerSecond,
                                           gravity,
                                           startScale,
                                           endScale,
                                           startRotationSpeed,
                                           endRotationSpeed,
                                           startRotationSpeedVariation,
                                           endRotationSpeedVariation);

    }// namespace

    tl::expected<ParticleSystem, std::string> ParticleSystem::loadFromFile(const std::filesystem::path& filename,
                                                                           const Texture& texture,
                                                                           ShaderProgram& shaderProgram) noexcept {
        using namespace JSONUtils;
        using namespace std::literals::string_literals;
        const auto fileContents = FileUtils::readTextFile(filename);
        auto json = nlohmann::json::parse(fileContents, nullptr, false);
        if (json.is_discarded()) {
            return tl::unexpected(fmt::format("Failed to parse JSON file {}", filename.string()));
        }
        auto particleSystemJSON = json.get<ParticleSystemJSON>();
        return ParticleSystem{ .texture{ &texture },
                               .shaderProgram{ &shaderProgram },
                               .startLifeTime{ particleSystemJSON.startLifeTime },
                               .lifeTimeVariation{ particleSystemJSON.lifeTimeVariation },
                               .particlesPerSecond{ particleSystemJSON.particlesPerSecond },
                               .gravity{ particleSystemJSON.gravity },
                               .startScale{ particleSystemJSON.startScale },
                               .endScale{ particleSystemJSON.endScale },
                               .startRotationSpeed{ particleSystemJSON.startRotationSpeed },
                               .endRotationSpeed{ particleSystemJSON.endRotationSpeed },
                               .startRotationSpeedVariation{ particleSystemJSON.startRotationSpeedVariation },
                               .endRotationSpeedVariation{ particleSystemJSON.endRotationSpeedVariation } };
    }
}// namespace c2k