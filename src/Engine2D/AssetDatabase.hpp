//
// Created by coder2k on 14.08.2021.
//

#pragma once

#include "GUID.hpp"
#include "Texture.hpp"
#include "ShaderProgram.hpp"
#include "SpriteSheet.hpp"
#include "AssetList.hpp"
#include "Script.hpp"
#include "ParticleSystem.hpp"
#include "Animation.hpp"
#include <algorithm>

namespace c2k {

    class AssetDatabase final {
    public:
        AssetDatabase() noexcept;
        void loadFromList(const AssetList& list) noexcept;
        void loadFromList(const std::filesystem::path& path) noexcept;

        [[nodiscard]] bool hasBeenLoaded(GUID guid) const noexcept {
            return mAssets.contains(guid);
        }

        bool unload(GUID guid) noexcept {
            const auto it = std::find_if(mAssets.begin(), mAssets.end(),
                                         [guid](const auto& pair) { return pair.first == guid; });
            if (it == mAssets.end()) {
                return false;
            }
            mAssets.erase(it);
            return true;
        }

        Texture& loadTexture(const std::filesystem::path& filename, GUID guid) noexcept {
            return load<Texture>(
                    guid,
                    [&]() {
                        return Image::loadFromFile(filename).and_then(Texture::create).map([&](Texture texture) {
                            texture.guid = guid;
                            return texture;
                        });
                    },
                    mDebugFallbackTexture);
        }

        ShaderProgram& loadShaderProgram(const std::filesystem::path& vertexShaderFilename,
                                         const std::filesystem::path& fragmentShaderFilename,
                                         GUID guid) noexcept {
            return load<ShaderProgram>(
                    guid,
                    [&]() {
                        return ShaderProgram::generateFromFiles(vertexShaderFilename, fragmentShaderFilename)
                                .map([&](ShaderProgram shaderProgram) {
                                    shaderProgram.guid = guid;
                                    return shaderProgram;
                                });
                    },
                    mDebugFallbackShaderProgram);
        }

        SpriteSheet& loadSpriteSheet(const std::filesystem::path& filename,
                                     GUID guid,
                                     const Texture& texture) noexcept {
            return load<SpriteSheet>(
                    guid, [&]() { return SpriteSheet::loadFromFile(filename, texture); }, mDebugFallbackSpriteSheet);
        }

        Script& loadScript(const std::filesystem::path& filename, GUID guid) noexcept {
            return load<Script>(
                    guid, [&filename, guid]() { return Script::loadFromFile(filename, guid); }, mDebugFallbackScript);
        }

        ParticleSystem& loadParticleSystem(const std::filesystem::path& filename,
                                           GUID guid,
                                           const Texture& texture,
                                           ShaderProgram& shaderProgram) noexcept {
            return load<ParticleSystem>(
                    guid, [&]() { return ParticleSystem::loadFromFile(filename, texture, shaderProgram, guid); },
                    mDebugFallbackParticleSystem);
        }

        Animation& loadAnimation(const std::filesystem::path& filename, GUID guid) noexcept {
            return load<Animation>(
                    guid, [&]() { return Animation::loadFromFile(filename); }, mDebugFallbackAnimation);
        }

        [[nodiscard]] Texture& textureMutable(GUID guid) noexcept {
            return getMutable<Texture>(guid, mDebugFallbackTexture);
        }

        [[nodiscard]] const Texture& texture(GUID guid) const noexcept {
            return get<Texture>(guid, mDebugFallbackTexture);
        }

        [[nodiscard]] Texture& fallbackTextureMutable() noexcept {
            return mDebugFallbackTexture;
        }

        [[nodiscard]] const Texture& fallbackTexture() const noexcept {
            return mDebugFallbackTexture;
        }

        [[nodiscard]] ShaderProgram& shaderProgramMutable(GUID guid) noexcept {
            return getMutable<ShaderProgram>(guid, mDebugFallbackShaderProgram);
        }

        [[nodiscard]] const ShaderProgram& shaderProgram(GUID guid) const noexcept {
            return get<ShaderProgram>(guid, mDebugFallbackShaderProgram);
        }

        [[nodiscard]] ShaderProgram& fallbackShaderProgramMutable() noexcept {
            return mDebugFallbackShaderProgram;
        }

        [[nodiscard]] const ShaderProgram& fallbackShaderProgram() const noexcept {
            return mDebugFallbackShaderProgram;
        }

        [[nodiscard]] const SpriteSheet& spriteSheet(GUID guid) const noexcept {
            return get<SpriteSheet>(guid, mDebugFallbackSpriteSheet);
        }

        [[nodiscard]] Script& scriptMutable(GUID guid) noexcept {
            return getMutable<Script>(guid, mDebugFallbackScript);
        }

        [[nodiscard]] const ParticleSystem& particleSystem(GUID guid) const noexcept {
            return get<ParticleSystem>(guid, mDebugFallbackParticleSystem);
        }

        [[nodiscard]] ParticleSystem& particleSystemMutable(GUID guid) noexcept {
            return getMutable<ParticleSystem>(guid, mDebugFallbackParticleSystem);
        }

        [[nodiscard]] const Animation& animation(GUID guid) noexcept {
            return get<Animation>(guid, mDebugFallbackAnimation);
        }

        static void setAssetPath(std::filesystem::path path) noexcept {
            sAssetPath = std::move(path);
        }

        [[nodiscard]] static auto assetPath() noexcept {
            return sAssetPath;
        }

        static void setWorkingDirectory(const std::filesystem::path& path) noexcept {
            std::filesystem::current_path(path);
        }

        [[nodiscard]] static auto workingDirectory() noexcept {
            return std::filesystem::current_path();
        }

        [[nodiscard]] static auto directoryIterator(const std::filesystem::path& path) noexcept {
            return std::filesystem::directory_iterator{ path };
        }

        [[nodiscard]] static auto recursiveDirectoryIterator(const std::filesystem::path& path) noexcept {
            return std::filesystem::recursive_directory_iterator{ path };
        }

    private:
        using Asset = std::variant<Texture, ShaderProgram, SpriteSheet, Script, ParticleSystem, Animation>;

    private:
        template<typename T, typename LoadFunc>
        T& load(GUID guid, LoadFunc&& loadFunc, T& debugFallback) noexcept {
            assert(!mAssets.contains(guid) && "This GUID is already taken.");
            if (auto expected = loadFunc()) {
                mAssets[guid] = std::move(expected.value());
#ifdef DEBUG_BUILD
                spdlog::info("Loaded asset for GUID {}", guid);
#endif
                return std::get<T>(mAssets[guid]);
            } else {
                spdlog::error("Could not load asset for GUID {}: {}", guid, expected.error());
                return debugFallback;
            }
        }

        template<typename T>
        [[nodiscard]] const T& get(GUID guid, const T& debugFallback) const noexcept {
            const auto findIterator = mAssets.find(guid);
            if (findIterator == mAssets.end() || !std::holds_alternative<T>(findIterator->second)) {
                spdlog::error("Unable to retrieve asset for GUID {}", guid);
                return debugFallback;
            }
            return std::get<T>(findIterator->second);
        }

        template<typename T>
        [[nodiscard]] T& getMutable(GUID guid, T& debugFallback) noexcept {
            const auto findIterator = mAssets.find(guid);
            if (findIterator == mAssets.end() || !std::holds_alternative<T>(findIterator->second)) {
                spdlog::error("Unable to retrieve asset for GUID {}", guid);
                return debugFallback;
            }
            return std::get<T>(findIterator->second);
        }

    private:
        static inline std::filesystem::path sAssetPath{ std::filesystem::current_path() / "assets" };

        std::unordered_map<GUID, Asset> mAssets;
        Texture mDebugFallbackTexture;
        ShaderProgram mDebugFallbackShaderProgram;
        SpriteSheet mDebugFallbackSpriteSheet;      // TODO: set
        Script mDebugFallbackScript;                // TODO: set
        ParticleSystem mDebugFallbackParticleSystem;// TODO: set
        Animation mDebugFallbackAnimation;
    };

}// namespace c2k
