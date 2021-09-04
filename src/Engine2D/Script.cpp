//
// Created by coder2k on 04.09.2021.
//

#include "Script.hpp"

namespace c2k {

    Script::Script(std::string source) noexcept {
        mLuaState.open_libraries(sol::lib::base);
        mLuaState.safe_script(source, [](lua_State*, sol::protected_function_result result) {
            sol::error err = result;
            spdlog::error("Error occurred: {}", err.what());
            return result;
        });
    }

    tl::expected<Script, std::string> Script::loadFromFile(const std::filesystem::path& filename) noexcept {
        return Script{ FileUtils::readTextFile(filename) };
    }

}// namespace c2k