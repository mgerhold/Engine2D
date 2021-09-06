//
// Created by coder2k on 30.08.2021.
//

#pragma once

namespace c2k {

    class Renderer;
    class Registry;

    struct ApplicationContext {
        ApplicationContext(Renderer& renderer, Registry& registry) noexcept;

        Renderer& renderer;
        Registry& registry;
    };

}// namespace c2k