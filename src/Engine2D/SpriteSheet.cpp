//
// Created by coder2k on 28.08.2021.
//

#include "SpriteSheet.hpp"
#include "FileUtils.hpp"
#include "JSONUtils.hpp"

namespace c2k {

    tl::expected<SpriteSheet, std::string> SpriteSheet::loadFromFile(const std::filesystem::path& filename,
                                                                     const Texture& texture) noexcept {
        using namespace JSONUtils;
        using namespace std::literals::string_literals;
        const auto fileContents = FileUtils::readTextFile(filename);
        auto json = nlohmann::json::parse(fileContents, nullptr, false);
        if (json.is_discarded()) {
            return tl::unexpected(fmt::format("Failed to parse JSON file {}", filename.string()));
        }
        constexpr auto metaKey = "meta";
        if (!validate(json, metaKey)) {
            return tl::unexpected("Sprite sheet must contain an object called \"meta\"."s);
        }
        // TODO: remove all this stuff and do it better

        int textureWidth;
        int textureHeight;
        json[metaKey]["size"]["w"].get_to(textureWidth);
        json[metaKey]["size"]["h"].get_to(textureHeight);
        constexpr auto framesKey = "frames";
        if (!validate(json, framesKey)) {
            return tl::unexpected("Sprite sheet must contain an array called \"frames\"."s);
        }
        auto jsonFrames = json[framesKey];
        if (!jsonFrames.is_array()) {
            return tl::unexpected("\"frames\" object must be an array."s);
        }
        SpriteSheet spriteSheet;
        spriteSheet.texture = &texture;
        for (const auto& jsonFrame : jsonFrames) {
            Frame frame;
            if (!validate(jsonFrame, "frame", "rotated", "sourceSize")) {
                return tl::unexpected(fmt::format("Missing keys within frames: {}", to_string(jsonFrame)));
            }
            auto jsonRect = jsonFrame["frame"];
            if (!validateNumbers(jsonRect, "x", "y", "w", "h")) {
                return tl::unexpected(
                        fmt::format("Missing keys or not a valid number within rect: {}", to_string(jsonRect)));
            }
            jsonRect["x"].get_to(frame.rect.left);
            jsonRect["y"].get_to(frame.rect.bottom);
            jsonRect["w"].get_to(frame.rect.right);
            jsonRect["h"].get_to(frame.rect.top);
            frame.rect.right += frame.rect.left;
            frame.rect.top += frame.rect.bottom;
            frame.rect.left /= gsl::narrow_cast<float>(textureWidth);
            frame.rect.bottom /= gsl::narrow_cast<float>(textureHeight);
            frame.rect.right /= gsl::narrow_cast<float>(textureWidth);
            frame.rect.top /= gsl::narrow_cast<float>(textureHeight);
            frame.rect.top = 1.0f - frame.rect.top;
            frame.rect.bottom = 1.0f - frame.rect.bottom;
            std::swap(frame.rect.top, frame.rect.bottom);
            auto jsonSourceSize = jsonFrame["sourceSize"];
            if (!validateNumbers(jsonSourceSize, "w", "h")) {
                return tl::unexpected(
                        fmt::format("Missing keys of invalid numbers in sourceSize: {}", to_string(jsonSourceSize)));
            }
            jsonSourceSize["w"].get_to(frame.sourceWidth);
            jsonSourceSize["h"].get_to(frame.sourceHeight);
            spriteSheet.frames.push_back(frame);
        }
        return spriteSheet;
    }

}// namespace c2k