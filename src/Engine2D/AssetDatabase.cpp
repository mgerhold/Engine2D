//
// Created by coder2k on 14.08.2021.
//

#include "AssetDatabase.hpp"

AssetDatabase::AssetDatabase() noexcept {
    auto expectedDebugTexture = Texture::createFromFillColor(1, 1, 3, Color{ .r{ 255 }, .g{ 0 }, .b{ 255 } });
    if (expectedDebugTexture) {
        mDebugFallbackTexture = std::move(expectedDebugTexture.value());
#ifdef DEBUG_BUILD
        spdlog::info("Created debug texture");
#endif
    } else {
        spdlog::error("Unable to create debug texture");
    }
}