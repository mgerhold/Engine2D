//
// Created by coder2k on 28.08.2021.
//

#pragma once

namespace c2k {

    struct Rect {
        float left;
        float bottom;
        float right;
        float top;

        [[nodiscard]] static Rect unit() noexcept {
            return Rect{ .left{ 0.0f }, .bottom{ 0.0f }, .right{ 1.0f }, .top{ 1.0f } };
        }
    };

}// namespace c2k