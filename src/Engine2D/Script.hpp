//
// Created by coder2k on 04.09.2021.
//

#pragma once

#include "FileUtils.hpp"
#include "expected/expected.hpp"

namespace c2k {

    class Script {
    public:
        explicit Script(std::string source) noexcept;

        [[nodiscard]] static tl::expected<Script, std::string> loadFromFile(
                const std::filesystem::path& filename) noexcept;

        template<typename... Args>
        void invoke(const std::string& functionName, Args... args) noexcept {
            auto result = mLuaState[functionName](args...);
            if (!result.valid()) {
                spdlog::error("Error while invoking function {}", functionName);
            }
        }

    private:
        sol::state mLuaState;
    };

}// namespace c2k
