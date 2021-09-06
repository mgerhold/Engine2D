//
// Created by coder2k on 06.09.2021.
//

#include "ApplicationContext.hpp"
#include "Script.hpp"

namespace c2k {

    ApplicationContext::ApplicationContext(Renderer& renderer, Registry& registry) noexcept
        : renderer{ renderer },
          registry{ registry } {
        Script::setApplicationContext(*this);
    }
}// namespace c2k