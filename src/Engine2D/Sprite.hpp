//
// Created by coder2k on 26.09.2021.
//

#pragma once

#include "Rect.hpp"
#include <cstddef>

namespace c2k {

    class Texture;
    struct SpriteSheet;

    struct Sprite {
        const Texture* texture;
        Rect textureRect;

        [[nodiscard]] static Sprite fromTexture(const Texture& texture) noexcept;
        [[nodiscard]] static Sprite fromSpriteSheet(const SpriteSheet& spriteSheet, std::size_t frameIndex) noexcept;
    };

}// namespace c2k