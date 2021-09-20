//
// Created by coder2k on 20.09.2021.
//

#pragma once

#include "Entity.hpp"
#include "GUID.hpp"
#include <variant>

namespace c2k {
    struct ApplicationContext;

    namespace BufferedScriptCommands {

        struct AttachScript {
            GUID scriptGUID{};
            Entity targetEntity{ invalidEntity };

            void process(ApplicationContext& appContext) const noexcept;
        };


        struct DestroyEntity {
            Entity targetEntity{ invalidEntity };

            void process(ApplicationContext& appContext) const noexcept;
        };

        using Command = std::variant<AttachScript, DestroyEntity>;

    }// namespace BufferedScriptCommands
}// namespace c2k