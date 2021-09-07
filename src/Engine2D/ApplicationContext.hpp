//
// Created by coder2k on 30.08.2021.
//

#pragma once

namespace c2k {

    class Renderer;
    class Registry;
    struct Time;
    class Input;

    struct ApplicationContext {
        ApplicationContext(Renderer& renderer, Registry& registry, Time& time, Input& input) noexcept;

        Renderer& renderer;
        Registry& registry;
        Time& time;
        Input& input;
    };

}// namespace c2k