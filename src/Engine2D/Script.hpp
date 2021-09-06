//
// Created by coder2k on 04.09.2021.
//

#pragma once

#include "FileUtils.hpp"
#include "expected/expected.hpp"
#include "ApplicationContext.hpp"

namespace c2k {

    class Script {
    public:
        explicit Script(std::string source) noexcept;

        [[nodiscard]] static tl::expected<Script, std::string> loadFromFile(
                const std::filesystem::path& filename) noexcept;

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

    private:
        std::unique_ptr<sol::state> mLuaState{ std::make_unique<sol::state>() };// indirection to keep references valid
        static inline ApplicationContext* sApplicationContext{ nullptr };
    };

}// namespace c2k
