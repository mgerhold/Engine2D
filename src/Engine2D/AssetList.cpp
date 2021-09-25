//
// Created by coder2k on 17.08.2021.
//

#include "AssetList.hpp"
#include "FileUtils/FileUtils.hpp"

namespace c2k {

    AssetList::AssetList(const std::filesystem::path& path) noexcept {
        fromFile(path);
    }

    void AssetList::fromFile(const std::filesystem::path& path) noexcept {
        const auto fileContents = FileUtils::readTextFile(path);
        auto json = nlohmann::json::parse(fileContents, nullptr, false);
        if (json.is_discarded()) {
            spdlog::error("Failed to parse JSON file {}", path.string());
            return;
        }
        mAssetDescriptions = json.get<AssetDescriptions::List>();
        spdlog::info("Parsed {} textures, {} shader programs, {} sprite sheets, {} scripts, {} particle systems",
                     mAssetDescriptions.textures.size(), mAssetDescriptions.shaderPrograms.size(),
                     mAssetDescriptions.spriteSheets.size(), mAssetDescriptions.scripts.size(),
                     mAssetDescriptions.particleSystems.size());
    }

}// namespace c2k