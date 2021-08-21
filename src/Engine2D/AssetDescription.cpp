//
// Created by coder2k on 19.08.2021.
//

#include "AssetDescription.hpp"

namespace c2k {

    template<typename... Keys>
    [[nodiscard]] bool validate(const nlohmann::json& json, Keys... keys) {
        return (json.contains(keys) && ...);
    }

    nlohmann::json TextureAssetDescription::serialize() noexcept {
        return nlohmann::json{ { "guid", guid.string() }, { "group", group }, { "filename", filename.string() } };
    }

    std::optional<TextureAssetDescription> TextureAssetDescription::deserialize(const nlohmann::json& json) noexcept {
        if (!validate(json, "guid", "group", "filename")) {
            return {};
        }
        return TextureAssetDescription{ .guid{ GUID::fromString(json["guid"].get<std::string>()) },
                                        .group{ json["group"].get<std::string>() },
                                        .filename{ std::filesystem::path{ json["filename"].get<std::string>() } } };
    }

    nlohmann::json ShaderProgramAssetDescription::serialize() noexcept {
        return nlohmann::json{ { "guid", guid.string() },
                               { "group", group },
                               { "vertexShaderFilename", vertexShaderFilename.string() },
                               { "fragmentShaderFilename", fragmentShaderFilename.string() } };
    }

    std::optional<ShaderProgramAssetDescription> ShaderProgramAssetDescription::deserialize(
            const nlohmann::json& json) noexcept {
        if (!validate(json, "guid", "group", "vertexShaderFilename", "fragmentShaderFilename")) {
            return {};
        }
        return ShaderProgramAssetDescription{
            .guid{ GUID::fromString(json["guid"].get<std::string>()) },
            .group{ json["group"].get<std::string>() },
            .vertexShaderFilename{ std::filesystem::path{ json["vertexShaderFilename"].get<std::string>() } },
            .fragmentShaderFilename{ std::filesystem::path{ json["fragmentShaderFilename"].get<std::string>() } }
        };
    }

}// namespace c2k