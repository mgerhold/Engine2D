//
// Created by coder2k on 25.09.2021.
//

#include "MathUtils.hpp"

namespace c2k::MathUtils {

    glm::vec2 lerp(const glm::vec2& a, const glm::vec2& b, float t) noexcept {
        return a * (1.0f - t) + b * t;
    }

    float lerp(float a, float b, float t) noexcept {
        return a * (1.0f - t) + b * t;
    }

    Color lerp(const Color& a, const Color& b, float t) noexcept {
        const auto result = a * (1.0f - t) + b * t;
        return Color{ result.x, result.y, result.z, result.w };
    };

}// namespace c2k::MathUtils
