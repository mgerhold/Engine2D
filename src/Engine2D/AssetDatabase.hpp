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

namespace c2k {

    class AssetDatabase final {
    public:
        AssetDatabase() noexcept;
        void loadFromList(const AssetList& list) noexcept;
        void loadFromList(const std::filesystem::path& path) noexcept;

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
                    guid, [&filename, &texture]() { return SpriteSheet::loadFromFile(filename, texture); },
                    mDebugFallbackSpriteSheet);
        }

        Script& loadScript(const std::filesystem::path& filename, GUID guid) noexcept {
            return load<Script>(
                    guid, [&filename, guid]() { return Script::loadFromFile(filename, guid); }, mDebugFallbackScript);
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

        [[nodiscard]] static auto assetPath() noexcept {
            return std::filesystem::current_path() / "assets";
        }

        [[nodiscard]] static auto directoryIterator(const std::filesystem::path& path) noexcept {
            return std::filesystem::directory_iterator{ path };
        }

        [[nodiscard]] static auto recursiveDirectoryIterator(const std::filesystem::path& path) noexcept {
            return std::filesystem::recursive_directory_iterator{ path };
        }

    private:
        using Asset = std::variant<Texture, ShaderProgram, SpriteSheet, Script>;

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
                spdlog::error("Could not load asset for GUID {}", guid, expected.error());
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
        std::unordered_map<GUID, Asset> mAssets;
        Texture mDebugFallbackTexture;
        ShaderProgram mDebugFallbackShaderProgram;// TODO: set
        SpriteSheet mDebugFallbackSpriteSheet;    // TODO: set
        Script mDebugFallbackScript;              // TODO: set
    };

}// namespace c2k
