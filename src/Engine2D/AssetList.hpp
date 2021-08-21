//
// Created by coder2k on 17.08.2021.
//

#pragma once

#include "GUID.hpp"
#include "AssetDescription.hpp"
#include "pch.hpp"

namespace c2k {

    class AssetList {
    public:
        AssetList() = default;
        explicit AssetList(const std::filesystem::path& path) noexcept;
        void fromFile(const std::filesystem::path& path) noexcept;
        [[nodiscard]] const auto& textureDescriptions() const noexcept {
            return mTextureDescriptions;
        }
        [[nodiscard]] const auto& shaderProgramDescriptions() const noexcept {
            return mShaderProgramDescriptions;
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
        std::vector<TextureAssetDescription> mTextureDescriptions;
        std::vector<ShaderProgramAssetDescription> mShaderProgramDescriptions;
    };

}