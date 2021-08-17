//
// Created by coder2k on 14.08.2021.
//

#pragma once

#include "GUID.hpp"
#include "Texture.hpp"
#include "ShaderProgram.hpp"
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <variant>
#include <filesystem>
#include <functional>
#include <cassert>

class AssetDatabase final {
public:
    AssetDatabase() noexcept;

    Texture& loadTexture(const std::filesystem::path& filename, GUID guid) noexcept {
        return load<Texture>(
                guid, [&filename]() { return Image::loadFromFile(filename).and_then(Texture::create); },
                mDebugFallbackTexture);
    }

    ShaderProgram& loadShaderProgram(const std::filesystem::path& vertexShaderFilename,
                                     const std::filesystem::path& fragmentShaderFilename,
                                     GUID guid) noexcept {
        return load<ShaderProgram>(
                guid,
                [&vertexShaderFilename, &fragmentShaderFilename]() {
                    return ShaderProgram::generateFromFiles(vertexShaderFilename, fragmentShaderFilename);
                },
                mDebugFallbackShaderProgram);
    }

    [[nodiscard]] const Texture& texture(GUID guid) const noexcept {
        return get<Texture>(guid, mDebugFallbackTexture);
    }
    [[nodiscard]] ShaderProgram& shaderProgramMutable(GUID guid) noexcept {
        return getMutable<ShaderProgram>(guid, mDebugFallbackShaderProgram);
    }
    [[nodiscard]] const ShaderProgram& shaderProgram(GUID guid) const noexcept {
        return get<ShaderProgram>(guid, mDebugFallbackShaderProgram);
    }

private:
    using Asset = std::variant<Texture, ShaderProgram>;

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
    ShaderProgram mDebugFallbackShaderProgram;
};
