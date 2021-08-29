//
// Created by coder2k on 17.08.2021.
//

#pragma once

#include "GUID.hpp"
#include "JSONUtils.hpp"

namespace c2k {

    namespace AssetDescriptions {
        struct TextureDescription {
            GUID guid;
            std::filesystem::path filename;
            std::string group;
        };

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextureDescription, guid, filename, group);

        struct ShaderProgramDescription {
            GUID guid;
            std::filesystem::path vertexShaderFilename;
            std::filesystem::path fragmentShaderFilename;
            std::string group;
        };

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ShaderProgramDescription,
                                           guid,
                                           vertexShaderFilename,
                                           fragmentShaderFilename,
                                           group);

        struct SpriteSheetDescription {
            GUID guid;
            std::filesystem::path filename;
            std::string group;
            GUID texture;
        };

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SpriteSheetDescription, guid, filename, group, texture);

        struct List {
            std::vector<TextureDescription> textures;
            std::vector<ShaderProgramDescription> shaderPrograms;
            std::vector<SpriteSheetDescription> spriteSheets;
        };

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(List, textures, shaderPrograms, spriteSheets);

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
        template<typename AssetDescription>
        void parseCategory(const nlohmann::json& json,
                           const std::string& title,
                           std::vector<AssetDescription>& targetContainer) {
            if (!json.contains(title)) {
                spdlog::warn("Asset list does not contain assets of category \"{}\"", title);
                return;
            }
            auto categoryItems = json[title];
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
        }

    private:
        AssetDescriptions::List mAssetDescriptions;
    };

}// namespace c2k