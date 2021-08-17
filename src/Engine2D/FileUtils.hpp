//
// Created by coder2k on 17.08.2021.
//

#pragma once

#include <filesystem>
#include <string>

namespace FileUtils {

    [[nodiscard]] std::string readTextFile(const std::filesystem::path& path) noexcept;

}