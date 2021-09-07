//
// Created by coder2k on 30.08.2021.
//

#pragma once

namespace c2k {

    class Renderer;
    class Registry;
    struct Time;

    struct ApplicationContext {
        ApplicationContext(Renderer& renderer, Registry& registry, Time& time) noexcept;

        Renderer& renderer;
        Registry& registry;
        Time& time;
    };

}// namespace c2k