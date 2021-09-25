//
// Created by coder2k on 25.09.2021.
//

#pragma once

namespace c2k::MathUtils {

    [[nodiscard]] glm::vec2 lerp(const glm::vec2& a, const glm::vec2& b, float t) noexcept;
    [[nodiscard]] float lerp(float a, float b, float t) noexcept;
}// namespace c2k::MathUtils