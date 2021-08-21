//
// Created by coder2k on 17.08.2021.
//

#pragma once

#include "pch.hpp"

namespace c2k::FileUtils {

    [[nodiscard]] std::string readTextFile(const std::filesystem::path& path) noexcept;

}