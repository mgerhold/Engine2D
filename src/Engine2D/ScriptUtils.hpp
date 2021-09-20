//
// Created by coder2k on 06.09.2021.
//

#pragma once

#include "ApplicationContext.hpp"
#include "Entity.hpp"
#include "GUID.hpp"

namespace c2k::ScriptUtils {

    struct LuaEntity {
        LuaEntity(Entity entity) noexcept : id{ entity } { }

        operator Entity() const noexcept {
            return id;
        }

        Entity id;
    };

    struct LuaTexture {
        std::string guid;
        int width;
        int height;
        int numChannels;
    };

    struct LuaShaderProgram {
        std::string guid;
    };

    struct LuaScript {
        std::string guid;
    };

    void provideAPI(ApplicationContext& applicationContext, sol::state& luaState) noexcept;

}// namespace c2k::ScriptUtils