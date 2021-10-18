//
// Created by coder2k on 17.08.2021.
//

#include "AssetList.hpp"
#include "FileUtils/FileUtils.hpp"
#include <spdlog/spdlog.h>

namespace c2k {

    AssetList::AssetList(const std::filesystem::path& path) noexcept {
        const auto result = fromFile(path);
        if (!result) {
            spdlog::error("Unable to read asset list: {}", result.error());
            return;
        }
        *this = result.value();
    }

    tl::expected<AssetList, std::string> AssetList::fromFile(const std::filesystem::path& path) noexcept {
        AssetList result;
        const auto parseResult = JSON::fromFile(path);
        if (!parseResult) {
            return tl::unexpected(fmt::format("Unable to parse asset list: {}", parseResult.error()));
        }
        const auto deserializationResult = parseResult->as(std::type_identity<AssetDescriptions::List>{});
        if (!deserializationResult) {
            return tl::unexpected(fmt::format("Unable to deserialize asset list: {}", deserializationResult.error()));
        }
        result.mAssetDescriptions = deserializationResult.value();
        return result;
    }

}// namespace c2k