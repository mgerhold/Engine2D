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
        const auto parseResult = JSON::fromFile(path);
        if (!parseResult) {
            spdlog::error("Unable to parse asset list: {}", parseResult.error());
            return;
        }
        const auto deserializationResult = parseResult->as(std::type_identity<AssetDescriptions::List>{});
        if (!deserializationResult) {
            spdlog::error("Unable to deserialize asset list");
            return;
        }
        mAssetDescriptions = deserializationResult.value();
    }

}// namespace c2k