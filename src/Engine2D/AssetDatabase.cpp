//
// Created by coder2k on 14.08.2021.
//

#include "AssetDatabase.hpp"

namespace c2k {

    AssetDatabase::AssetDatabase() noexcept {
        auto expectedDebugTexture = Texture::createFromFillColor(1, 1, 3, Color{ 1.0f, 0.0f, 1.0f, 1.0f });
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
        for (const auto& particleSystemDescription : list.assetDescriptions().particleSystems) {
            loadParticleSystem(assets / particleSystemDescription.filename, particleSystemDescription.guid,
                               texture(particleSystemDescription.texture),
                               shaderProgramMutable(particleSystemDescription.shaderProgram));
        }
    }

    void AssetDatabase::loadFromList(const std::filesystem::path& path) noexcept {
        loadFromList(AssetList{ path });
    }

}// namespace c2k