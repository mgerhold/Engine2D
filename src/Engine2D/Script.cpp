//
// Created by coder2k on 04.09.2021.
//

#include "Script.hpp"
#include "ScriptUtils/ScriptUtils.hpp"
#include "AssetDatabase.hpp"
#include <filesystem>

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

    Script::Script(std::string source, GUID guid, const std::filesystem::path& scriptBaseDirectory) noexcept
        : guid{ guid } {
        assert(sApplicationContext != nullptr &&
               "Scripts can only be instantiated after setting the application context.");
        mLuaState->open_libraries(sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::package, sol::lib::string);
        const std::string packagePath = (*mLuaState)["package"]["path"];
        const auto path = (scriptBaseDirectory.empty() ? std::string{ "" } : (scriptBaseDirectory / "?.lua").string());
        (*mLuaState)["package"]["path"] = packagePath + (!packagePath.empty() ? ";" : "") + path;
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
            return Script{ fileContents, guid, filename.parent_path() };
        });
    }

    void Script::setApplicationContext(ApplicationContext& context) noexcept {
        sApplicationContext = &context;
    }

}// namespace c2k