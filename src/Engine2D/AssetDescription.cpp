//
// Created by coder2k on 19.08.2021.
//

#include "AssetDescription.hpp"
#include "JSONUtils.hpp"

namespace c2k {

    nlohmann::json TextureAssetDescription::serialize() noexcept {
        return nlohmann::json{ { "guid", guid.string() }, { "group", group }, { "filename", filename.string() } };
    }

    std::optional<TextureAssetDescription> TextureAssetDescription::deserialize(const nlohmann::json& json) noexcept {
        using namespace JSONUtils;
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
        using namespace JSONUtils;
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

    std::optional<SpriteSheetAssetDescription> SpriteSheetAssetDescription::deserialize(
            const nlohmann::json& json) noexcept {
        using namespace JSONUtils;
        if (!validate(json, "guid", "group", "filename", "texture")) {
            return {};
        }
        return SpriteSheetAssetDescription{ .guid{ GUID::fromString(json["guid"].get<std::string>()) },
                                            .group{ json["group"].get<std::string>() },
                                            .filename{ std::filesystem::path{ json["filename"].get<std::string>() } },
                                            .texture{ GUID::fromString(json["texture"].get<std::string>()) } };
    }
}// namespace c2k