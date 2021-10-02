//
// Created by coder2k on 17.08.2021.
//

#pragma once

#include "GUID.hpp"
#include "JSONUtils.hpp"
#include "JSON/JSON.hpp"

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

        struct List {
            std::vector<TextureDescription> textures;
            std::vector<ShaderProgramDescription> shaderPrograms;
            std::vector<SpriteSheetDescription> spriteSheets;
            std::vector<ScriptDescription> scripts;
            std::vector<ParticleSystemDescription> particleSystems;
        };

        C2K_JSON_DEFINE_TYPE(List, textures, shaderPrograms, spriteSheets, scripts, particleSystems);

    }// namespace AssetDescriptions

    class AssetList {
    public:
        AssetList() = default;
        explicit AssetList(const std::filesystem::path& path) noexcept;
        void fromFile(const std::filesystem::path& path) noexcept;
        [[nodiscard]] const AssetDescriptions::List& assetDescriptions() const noexcept {
            return mAssetDescriptions;
        }

    private:
        /*template<typename AssetDescription>
        void parseCategory(const JSON::Value& json,
                           const std::string& title,
                           std::vector<AssetDescription>& targetContainer) {
            if (!json.containsKey(title)) {
                spdlog::warn("Asset list does not contain assets of category \"{}\"", title);
                return;
            }
            auto categoryItems = json.at(title);
            if (!categoryItems.is_array()) {
                spdlog::error("Items of category \"{}\" must be an array", title);
                return;
            }
            for (const auto& item : categoryItems) {
                const auto parsed = AssetDescription::deserialize(item);
                if (!parsed) {
                    spdlog::error("Failed to parse item: {}", to_string(item));
                    continue;
                }
                targetContainer.push_back(parsed.value());
            }
            spdlog::info("Parsed {} items of category \"{}\"...", targetContainer.size(), title);
        }*/

    private:
        AssetDescriptions::List mAssetDescriptions;
    };

}// namespace c2k