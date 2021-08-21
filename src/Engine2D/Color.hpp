//
// Created by coder2k on 14.08.2021.
//

#pragma once

#include "pch.hpp"

namespace c2k {

    struct Color {
        using Channel = std::uint8_t;

        Channel r{ 0 }, g{ 0 }, b{ 0 }, a{ 255 };

        [[nodiscard]] auto normalized() const noexcept {
            return glm::vec4{ static_cast<float>(r) / static_cast<float>(std::numeric_limits<Channel>::max()),
                              static_cast<float>(g) / static_cast<float>(std::numeric_limits<Channel>::max()),
                              static_cast<float>(b) / static_cast<float>(std::numeric_limits<Channel>::max()),
                              static_cast<float>(a) / static_cast<float>(std::numeric_limits<Channel>::max()) };
        };

        static constexpr Color white() {
            return Color{ 255, 255, 255, 255 };
        }
    };

}// namespace c2k