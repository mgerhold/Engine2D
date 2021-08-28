//
// Created by coder2k on 19.08.2021.
//

#pragma once

#include "GUID.hpp"
#include "pch.hpp"

namespace c2k {

    struct TextureAssetDescription {
        GUID guid;
        std::string group;
        std::filesystem::path filename;

        [[nodiscard]] nlohmann::json serialize() noexcept;
        [[nodiscard]] static std::optional<TextureAssetDescription> deserialize(const nlohmann::json& json) noexcept;
    };

    struct ShaderProgramAssetDescription {
        GUID guid;
        std::string group;
        std::filesystem::path vertexShaderFilename;
        std::filesystem::path fragmentShaderFilename;

        [[nodiscard]] nlohmann::json serialize() noexcept;
        [[nodiscard]] static std::optional<ShaderProgramAssetDescription> deserialize(
                const nlohmann::json& json) noexcept;
    };

    struct SpriteSheetAssetDescription {
        GUID guid;
        std::string group;
        std::filesystem::path filename;
        GUID texture;

        [[nodiscard]] static std::optional<SpriteSheetAssetDescription> deserialize(
                const nlohmann::json& json) noexcept;
    };

}// namespace c2k