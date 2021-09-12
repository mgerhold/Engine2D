//
// Created by coder2k on 06.09.2021.
//

#include "ApplicationContext.hpp"
#include "Script.hpp"

namespace c2k {

    ApplicationContext::ApplicationContext(Renderer& renderer,
                                           Registry& registry,
                                           Time& time,
                                           Input& input,
                                           AssetDatabase& assetDatabase) noexcept
        : renderer{ renderer },
          registry{ registry },
          time{ time },
          input{ input },
          assetDatabase{ assetDatabase } {
        Script::setApplicationContext(*this);
    }
}// namespace c2k