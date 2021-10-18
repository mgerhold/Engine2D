//
// Created by coder2k on 17.08.2021.
//

#pragma once

#include "GUID.hpp"
#include "JSONUtils.hpp"
#include "JSON/JSON.hpp"
#include <tl/expected.hpp>

namespace c2k {

    namespace AssetDescriptions {
        struct TextureDescription {
            GUID guid;
            std::filesystem::path filename;
            std::string group;
        };

        C2K_JSON_DEFINE_TYPE(TextureDescription, guid, filename, group);

        struct ShaderProgramDescription {
            GUID guid;
            std::filesystem::path vertexShaderFilename;
            std::filesystem::path fragmentShaderFilename;
            std::string group;
        };

        C2K_JSON_DEFINE_TYPE(ShaderProgramDescription, guid, vertexShaderFilename, fragmentShaderFilename, group);

        struct SpriteSheetDescription {
            GUID guid;
            std::filesystem::path filename;
            std::string group;
            GUID texture;
        };

        C2K_JSON_DEFINE_TYPE(SpriteSheetDescription, guid, filename, group, texture);

        struct ScriptDescription {
            GUID guid;
            std::filesystem::path filename;
            std::string group;
        };

        C2K_JSON_DEFINE_TYPE(ScriptDescription, guid, filename, group);

        struct ParticleSystemDescription {
            GUID guid;
            std::filesystem::path filename;
            std::string group;
            GUID texture;
            GUID shaderProgram;
        };

        C2K_JSON_DEFINE_TYPE(ParticleSystemDescription, guid, filename, group, texture, shaderProgram);

        struct AnimationDescription {
            GUID guid;
            std::filesystem::path filename;
        };

        C2K_JSON_DEFINE_TYPE(AnimationDescription, guid, filename);

        struct List {
            std::optional<std::vector<TextureDescription>> textures;
            std::optional<std::vector<ShaderProgramDescription>> shaderPrograms;
            std::optional<std::vector<SpriteSheetDescription>> spriteSheets;
            std::optional<std::vector<ScriptDescription>> scripts;
            std::optional<std::vector<ParticleSystemDescription>> particleSystems;
            std::optional<std::vector<AnimationDescription>> animations;
        };

        C2K_JSON_DEFINE_TYPE(List, textures, shaderPrograms, spriteSheets, scripts, particleSystems, animations);

    }// namespace AssetDescriptions

    class AssetList {
    public:
        AssetList() = default;
        explicit AssetList(const std::filesystem::path& path) noexcept;
        [[nodiscard]] const AssetDescriptions::List& assetDescriptions() const noexcept {
            return mAssetDescriptions;
        }
        [[nodiscard]] static tl::expected<AssetList, std::string> fromFile(const std::filesystem::path& path) noexcept;

    private:
        AssetDescriptions::List mAssetDescriptions;
    };

}// namespace c2k