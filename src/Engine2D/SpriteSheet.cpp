//
// Created by coder2k on 28.08.2021.
//

#include "SpriteSheet.hpp"
#include "FileUtils/FileUtils.hpp"
#include "JSONUtils.hpp"
#include "JSON/JSON.hpp"

namespace c2k {

    struct SizeJSON {
        int w;
        int h;
    };

    C2K_JSON_DEFINE_TYPE(SizeJSON, w, h);

    struct RectJSON {
        int x;
        int y;
        int w;
        int h;
    };

    C2K_JSON_DEFINE_TYPE(RectJSON, x, y, w, h);

    struct FrameJSON {
        RectJSON frame;
        SizeJSON sourceSize;
    };

    C2K_JSON_DEFINE_TYPE(FrameJSON, frame, sourceSize);

    struct MetaJSON {
        SizeJSON size;
    };

    C2K_JSON_DEFINE_TYPE(MetaJSON, size);

    struct SpriteSheetJSON {
        std::vector<FrameJSON> frames;
        MetaJSON meta;
    };

    C2K_JSON_DEFINE_TYPE(SpriteSheetJSON, frames, meta);

    tl::expected<SpriteSheet, std::string> SpriteSheet::loadFromFile(const std::filesystem::path& filename,
                                                                     const Texture& texture) noexcept {
        using namespace JSONUtils;
        using namespace std::literals::string_literals;
        const auto readResult = JSON::fromFile(filename).and_then(JSON::as<SpriteSheetJSON>);
        if (!readResult) {
            return tl::unexpected(
                    fmt::format("Unable to read sprite sheet information from JSON: {}", readResult.error()));
        }
        const auto spriteSheetJSON = readResult.value();
        const float textureWidth = static_cast<float>(spriteSheetJSON.meta.size.w);
        const float textureHeight = static_cast<float>(spriteSheetJSON.meta.size.h);
        SpriteSheet spriteSheet;
        spriteSheet.texture = &texture;
        for (const auto& frameJSON : spriteSheetJSON.frames) {
            Frame frame;
            frame.rect.left = static_cast<float>(frameJSON.frame.x) / textureWidth;
            frame.rect.right = static_cast<float>(frameJSON.frame.w + frameJSON.frame.x) / textureWidth;
            frame.rect.bottom = 1.0f - static_cast<float>(frameJSON.frame.h + frameJSON.frame.y) / textureHeight;
            frame.rect.top = 1.0f - static_cast<float>(frameJSON.frame.y) / textureHeight;
            frame.sourceWidth = frameJSON.sourceSize.w;
            frame.sourceHeight = frameJSON.sourceSize.h;
            spriteSheet.frames.push_back(frame);
        }
        return spriteSheet;
    }

}// namespace c2k