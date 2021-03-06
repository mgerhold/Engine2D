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
        mDebugFallbackAnimation = Animation::fromFramerate(50.0);
        mDebugFallbackShaderProgram = ShaderProgram::defaultProgram();
    }

    void AssetDatabase::loadFromList(const AssetList& list) noexcept {
        const auto assets = assetPath();
        if (list.assetDescriptions().textures) {
            for (const auto& textureDescription : list.assetDescriptions().textures.value()) {
                loadTexture(assets / textureDescription.filename, textureDescription.guid);
            }
        }
        if (list.assetDescriptions().shaderPrograms) {
            for (const auto& shaderProgramDescription : list.assetDescriptions().shaderPrograms.value()) {
                loadShaderProgram(assets / shaderProgramDescription.vertexShaderFilename,
                                  assets / shaderProgramDescription.fragmentShaderFilename,
                                  shaderProgramDescription.guid);
            }
        }
        if (list.assetDescriptions().spriteSheets) {
            for (const auto& spriteSheetDescription : list.assetDescriptions().spriteSheets.value()) {
                loadSpriteSheet(assets / spriteSheetDescription.filename, spriteSheetDescription.guid,
                                texture(spriteSheetDescription.texture));
            }
        }
        if (list.assetDescriptions().scripts) {
            for (const auto& scriptDescription : list.assetDescriptions().scripts.value()) {
                loadScript(assets / scriptDescription.filename, scriptDescription.guid);
            }
        }
        if (list.assetDescriptions().particleSystems) {
            for (const auto& particleSystemDescription : list.assetDescriptions().particleSystems.value()) {
                loadParticleSystem(assets / particleSystemDescription.filename, particleSystemDescription.guid,
                                   texture(particleSystemDescription.texture),
                                   shaderProgramMutable(particleSystemDescription.shaderProgram));
            }
        }
        if (list.assetDescriptions().animations) {
            for (const auto& animationDescription : list.assetDescriptions().animations.value()) {
                loadAnimation(assets / animationDescription.filename, animationDescription.guid);
            }
        }
    }

    void AssetDatabase::loadFromList(const std::filesystem::path& path) noexcept {
        loadFromList(AssetList{ path });
    }

}// namespace c2k