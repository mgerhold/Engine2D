//
// Created by coder2k on 30.08.2021.
//

#pragma once

#include "Entity.hpp"
#include "BufferedScriptCommand.hpp"
#include <vector>

namespace c2k {

    class Renderer;
    class Registry;
    struct Time;
    class Input;
    class AssetDatabase;
    class Application;

    struct ApplicationContext {
        ApplicationContext(Renderer& renderer,
                           Registry& registry,
                           Time& time,
                           Input& input,
                           AssetDatabase& assetDatabase,
                           Application& application,
                           Entity mainCameraEntity) noexcept;

        Renderer& renderer;
        Registry& registry;
        Time& time;
        Input& input;
        AssetDatabase& assetDatabase;
        Application& application;
        Entity mainCameraEntity;
        std::vector<BufferedScriptCommands::Command> bufferedScriptCommands;
    };

}// namespace c2k