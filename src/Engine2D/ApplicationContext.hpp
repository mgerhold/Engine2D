//
// Created by coder2k on 30.08.2021.
//

#pragma once

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
                           AssetDatabase& assetDatabase) noexcept;

        Renderer& renderer;
        Registry& registry;
        Time& time;
        Input& input;
        AssetDatabase& assetDatabase;
    };

}// namespace c2k