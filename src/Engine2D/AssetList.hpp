//
// Created by coder2k on 17.08.2021.
//

#pragma once

#include "GUID.hpp"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

class AssetList {
public:
    AssetList() = default;
    AssetList(const std::filesystem::path& path) noexcept;

    void fromFile(const std::filesystem::path& path) noexcept;

private:
    struct TextureAssetDescription {
        GUID guid;
        std::optional<std::string> group;
        std::filesystem::path filename;
    };

    struct ShaderProgramAssetDescription {
        GUID guid;
        std::optional<std::string> group;
        std::filesystem::path vertexShaderFilename;
        std::filesystem::path fragmentShaderFilename;
    };

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
            const auto parsed = convertFromJSON<AssetDescription>(item);
            if (!parsed) {
                spdlog::error("Failed to parse item: {}", to_string(item));
                continue;
            }
            targetContainer.push_back(parsed.value());
        }
        spdlog::info("Parsed {} items of category \"{}\"...", targetContainer.size(), title);
    }

    template<typename Target>
    [[nodiscard]] std::optional<Target> convertFromJSON(const nlohmann::json& json) {
        Target::unimplemented_function;
    }

    template<>
    [[nodiscard]] std::optional<TextureAssetDescription> convertFromJSON(const nlohmann::json& json) {
        if (!verifyStringKeys(json, "guid", "filename")) {
            return {};
        }
        return TextureAssetDescription{
            .guid{ GUID::fromString(json["guid"].get<std::string>()) },
            .group{ verifyStringKey(json, "group") ? std::optional<std::string>(json["group"])
                                                   : std::optional<std::string>{} },
            .filename{ json["filename"].get<std::string>() },
        };
    }

    template<>
    [[nodiscard]] std::optional<ShaderProgramAssetDescription> convertFromJSON(const nlohmann::json& json) {
        if (!verifyStringKeys(json, "guid", "vertexShaderFilename", "fragmentShaderFilename")) {
            return {};
        }
        return ShaderProgramAssetDescription{
            .guid{ GUID::fromString(json["guid"].get<std::string>()) },
            .group{ verifyStringKey(json, "group") ? std::optional<std::string>(json["group"])
                                                   : std::optional<std::string>{} },
            .vertexShaderFilename{ json["vertexShaderFilename"].get<std::string>() },
            .fragmentShaderFilename{ json["fragmentShaderFilename"].get<std::string>() }
        };
    }

    template<typename... Keys>
    [[nodiscard]] inline bool verifyStringKeys(const nlohmann::json& json, Keys... keys) noexcept {
        return (verifyStringKey(json, keys) && ...);
    }

    [[nodiscard]] inline bool verifyStringKey(const nlohmann::json& json, const std::string& key) noexcept {
        return json.contains(key) && json[key].is_string();
    }

private:
    std::vector<TextureAssetDescription> mTextureDescriptions;
    std::vector<ShaderProgramAssetDescription> mShaderProgramDescriptions;
};
