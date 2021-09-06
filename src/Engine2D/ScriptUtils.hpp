//
// Created by coder2k on 06.09.2021.
//

#pragma once

#include "ApplicationContext.hpp"

namespace c2k::ScriptUtils {

    void registerTypes(sol::state& luaState) noexcept;
    void provideAPI(ApplicationContext& applicationContext, sol::state& luaState) noexcept;

}// namespace c2k::ScriptUtils