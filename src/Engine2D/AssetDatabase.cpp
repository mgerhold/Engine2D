//
// Created by coder2k on 14.08.2021.
//

#include "AssetDatabase.hpp"

namespace c2k {

    AssetDatabase::AssetDatabase() noexcept : mDebugFallbackScript{ "" } {// TODO: load actual fallback script
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

    void AssetDatabase::loadFromList(const AssetList& list) noexcept {
        const auto assets = assetPath();
        for (const auto& textureDescription : list.assetDescriptions().textures) {
            loadTexture(assets / textureDescription.filename, textureDescription.guid);
        }
        for (const auto& shaderProgramDescription : list.assetDescriptions().shaderPrograms) {
            loadShaderProgram(assets / shaderProgramDescription.vertexShaderFilename,
                              assets / shaderProgramDescription.fragmentShaderFilename, shaderProgramDescription.guid);
        }
        for (const auto& spriteSheetDescription : list.assetDescriptions().spriteSheets) {
            loadSpriteSheet(assets / spriteSheetDescription.filename, spriteSheetDescription.guid,
                            texture(spriteSheetDescription.texture));
        }
        for (const auto& scriptDescription : list.assetDescriptions().scripts) {
            loadScript(assets / scriptDescription.filename, scriptDescription.guid);
        }
    }

    void AssetDatabase::loadFromList(const std::filesystem::path& path) noexcept {
        loadFromList(AssetList{ path });
    }

}// namespace c2k