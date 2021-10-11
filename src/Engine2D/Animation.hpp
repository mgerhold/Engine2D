//
// Created by coder2k on 11.10.2021.
//

#pragma once

#include "JSON/JSON.hpp"
#include <tl/expected.hpp>
#include <variant>
#include <vector>
#include <filesystem>

namespace c2k {

    namespace {
        struct AnimationJSON {
            std::optional<double> framesPerSecond;
            std::optional<std::vector<double>> frameTimes;
        };

        C2K_JSON_DEFINE_TYPE(AnimationJSON, framesPerSecond, frameTimes);
    }// namespace

    struct Animation : public std::variant<double, std::vector<double>> {

        using std::variant<double, std::vector<double>>::variant;

        [[nodiscard]] static inline tl::expected<Animation, std::string> loadFromFile(
                const std::filesystem::path& filename) noexcept {
            const auto deserializationResult = JSON::fromFile(filename).and_then(JSON::as<AnimationJSON>);
            if (!deserializationResult) {
                return tl::unexpected(deserializationResult.error());
            }
            const auto& deserialized = deserializationResult.value();
            if (deserialized.framesPerSecond) {
                return Animation{ deserialized.framesPerSecond.value() };
            }
            if (deserialized.frameTimes) {
                return Animation{ deserialized.frameTimes.value() };
            }
            return tl::unexpected(fmt::format(
                    "Failed to load animation {}: Either framesPerSecond of frameTimes array must be given.",
                    filename.string()));
        }

        [[nodiscard]] static inline Animation fromFramerate(double framerate) noexcept {
            assert(framerate > 0.0);
            return Animation{ framerate };
        }
    };

}// namespace c2k