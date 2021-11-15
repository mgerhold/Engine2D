//
// Created by coder2k on 17.08.2021.
//

#pragma once

#include <tl/expected.hpp>
#include <filesystem>
#include <string>
#include <string_view>

namespace c2k::FileUtils {

    [[nodiscard]] tl::expected<std::string, std::string> readTextFile(const std::filesystem::path& path) noexcept;
    [[nodiscard]] tl::expected<std::monostate, std::string> writeTextFile(const std::string& text,
                                                                          const std::filesystem::path& path) noexcept;
    [[nodiscard]] tl::expected<std::filesystem::path, std::string> findFileInParent(
            const std::filesystem::path& parentPath,
            std::string_view filename) noexcept;

}// namespace c2k::FileUtils