//
// Created by coder2k on 28.08.2021.
//

#pragma once

#include "Texture.hpp"
#include "Rect.hpp"

namespace c2k {

    struct SpriteSheet {
        struct Frame {
            Rect rect;
            int sourceWidth;
            int sourceHeight;

            [[nodiscard]] float getWidthToHeightRatio() const noexcept {
                return static_cast<float>(sourceWidth) / static_cast<float>(sourceHeight);
            }
        };

        std::vector<Frame> frames;
        const Texture* texture{ nullptr };

        [[nodiscard]] static tl::expected<SpriteSheet, std::string> loadFromFile(const std::filesystem::path& filename,
                                                                                 const Texture& texture) noexcept;
    };

}// namespace c2k