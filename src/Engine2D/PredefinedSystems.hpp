//
// Created by coder2k on 19.08.2021.
//

#pragma once

#include "Entity.hpp"
#include "Renderer.hpp"
#include "Component.hpp"

namespace DynamicSpriteRenderer {

    inline void init(Renderer& renderer, const Transform& cameraTransform) noexcept {
        renderer.clear(true, true);
        renderer.beginFrame(Camera::viewMatrix(cameraTransform));
    }

    inline void forEach(Renderer& renderer, Entity, const DynamicSprite& sprite, const Transform& transform) {
        renderer.drawQuad(transform.position, transform.rotation, transform.scale, *sprite.shader, *sprite.texture,
                          sprite.color);
    }

    inline void finalize(Renderer& renderer) {
        renderer.endFrame();
    }

}// namespace DynamicSpriteRenderer
