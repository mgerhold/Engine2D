//
// Created by coder2k on 17.08.2021.
//

#pragma once

namespace c2k::FileUtils {

    [[nodiscard]] tl::expected<std::string, std::string> readTextFile(const std::filesystem::path& path) noexcept;

}