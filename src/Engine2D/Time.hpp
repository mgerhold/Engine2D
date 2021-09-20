//
// Created by coder2k on 16.08.2021.
//

#pragma once

namespace c2k {

    struct Time {
        double elapsed{ 1.0 / 60.0 };
        double delta{ 1.0 / 60.0 };
        double meanFramesPerSecond{ 1.0 / 60.0 };

        [[nodiscard]] double meanFrameTime() const noexcept {
            return 1.0 / meanFramesPerSecond;
        }
    };

}// namespace c2k