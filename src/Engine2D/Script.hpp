//
// Created by coder2k on 04.09.2021.
//

#pragma once

#include "FileUtils/FileUtils.hpp"
#include "ScriptUtils/ScriptUtils.hpp"
#include "Entity.hpp"
#include "ApplicationContext.hpp"
#include "GUID.hpp"
#include <sol/sol.hpp>
#include <memory>

namespace c2k {

    class Script {
    public:
        Script() noexcept;
        explicit Script(std::string source, GUID guid) noexcept;

        [[nodiscard]] static tl::expected<Script, std::string> loadFromFile(const std::filesystem::path& filename,
                                                                            GUID guid = GUID{}) noexcept;

        static void setApplicationContext(ApplicationContext& context) noexcept;

        template<typename... Args>
        void invoke(const std::string& functionName, Args... args) noexcept {
            sol::protected_function function = mLuaState->get<sol::protected_function>(functionName);
            if (!function.valid()) {
                spdlog::error("{} is not a valid function", functionName);
                return;
            }
            sol::protected_function_result result = function(args...);
            if (!result.valid()) {
                sol::error error = result;
                spdlog::error("Error while invoking function {}: {}", functionName, error.what());
                return;
            }
        }

        void invokeOnAttach(ScriptUtils::LuaEntity luaEntity) noexcept {
            if (mOnAttachFunction.valid()) {
                sol::protected_function_result result = mOnAttachFunction(luaEntity);
                if (!result.valid()) {
                    sol::error error = result;
                    spdlog::error("Error executing onAttach()-function: {}", error.what());
                }
            }
        }

        void invokeUpdate(ScriptUtils::LuaEntity luaEntity) noexcept {
            if (mUpdateFunction.valid()) {
                sol::protected_function_result result = mUpdateFunction(luaEntity);
                if (!result.valid()) {
                    sol::error error = result;
                    spdlog::error("Update function on script is invalid: {}", error.what());
                }
            }
        }

    public:
        GUID guid;

    private:
        std::unique_ptr<sol::state> mLuaState{ std::make_unique<sol::state>() };// indirection to keep references valid
        sol::protected_function mOnAttachFunction;
        sol::protected_function mUpdateFunction;
        static inline ApplicationContext* sApplicationContext{ nullptr };
    };

}// namespace c2k
