//
// Created by coder2k on 26.09.2021.
//

#include "Sprite.hpp"
#include "SpriteSheet.hpp"
#include <memory>
#include <vector>

namespace c2k {

    Sprite Sprite::fromTexture(const Texture& texture) noexcept {
        return Sprite{ .texture{ &texture }, .textureRect{ Rect::unit() } };
    }

    Sprite Sprite::fromSpriteSheet(const SpriteSheet& spriteSheet, std::size_t frameIndex) noexcept {
        return Sprite{ .texture{ spriteSheet.texture }, .textureRect{ spriteSheet.frames[frameIndex].rect } };
    }

}// namespace c2k