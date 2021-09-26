//
// Created by coder2k on 04.09.2021.
//

#include "Script.hpp"
#include "ScriptUtils/ScriptUtils.hpp"

namespace c2k {

    Script::Script() noexcept {
        mLuaState->open_libraries(sol::lib::base);
        mLuaState->script(R"(function onAttach(entity)
printWarning(entity)
end

function update(entity)
printWarning(entity)
end

function printWarning(entity)
print("Warning: Default script on entity " .. entity.id)
end)");
    }

    Script::Script(std::string source, GUID guid) noexcept : guid{ guid } {
        assert(sApplicationContext != nullptr &&
               "Scripts can only be instantiated after setting the application context.");
        mLuaState->open_libraries(sol::lib::base, sol::lib::math);
        ScriptUtils::provideAPI(*sApplicationContext, *mLuaState);
        mLuaState->safe_script(source, [](lua_State*, sol::protected_function_result result) {
            sol::error err = result;
            spdlog::error("Error occurred: {}", err.what());
            return result;
        });
        mOnAttachFunction = mLuaState->get<sol::protected_function>("onAttach");
        mUpdateFunction = mLuaState->get<sol::protected_function>("update");
    }

    tl::expected<Script, std::string> Script::loadFromFile(const std::filesystem::path& filename, GUID guid) noexcept {
        return FileUtils::readTextFile(filename).transform([&](std::string&& fileContents) {
            return Script{ fileContents, guid };
        });
    }

    void Script::setApplicationContext(ApplicationContext& context) noexcept {
        sApplicationContext = &context;
    }

}// namespace c2k