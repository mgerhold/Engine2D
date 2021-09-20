//
// Created by coder2k on 30.08.2021.
//

#pragma once

#include "Entity.hpp"

namespace c2k {

    class Renderer;
    class Registry;
    struct Time;
    class Input;
    class AssetDatabase;

    struct ApplicationContext {
        ApplicationContext(Renderer& renderer,
                           Registry& registry,
                           Time& time,
                           Input& input,
                           AssetDatabase& assetDatabase,
                           Entity mainCameraEntity) noexcept;

        Renderer& renderer;
        Registry& registry;
        Time& time;
        Input& input;
        AssetDatabase& assetDatabase;
        Entity mainCameraEntity;
    };

}// namespace c2k