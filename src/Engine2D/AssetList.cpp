//
// Created by coder2k on 17.08.2021.
//

#include "AssetList.hpp"
#include "FileUtils.hpp"

namespace c2k {

    AssetList::AssetList(const std::filesystem::path& path) noexcept {
        fromFile(path);
    }

    void AssetList::fromFile(const std::filesystem::path& path) noexcept {
        const auto fileContents = FileUtils::readTextFile(path);
        auto json = nlohmann::json::parse(fileContents, nullptr, false);
        if (json.is_discarded()) {
            spdlog::error("Failed to parse JSON file {}: {}", path.string());
            return;
        }
        spdlog::info("Parsing asset list {}...", path.string());
        parseCategory(json, "textures", mTextureDescriptions);
        parseCategory(json, "shaderPrograms", mShaderProgramDescriptions);
    }

}