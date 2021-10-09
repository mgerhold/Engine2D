//
// Created by coder2k on 14.08.2021.
//

#pragma once

#include <glm/glm.hpp>

namespace c2k {

    struct Color : public glm::vec4 {
        using glm::vec4::vec4;

        static constexpr Color white() {
            return Color{ 1.0f, 1.0f, 1.0f, 1.0f };
        }
    };

}// namespace c2k