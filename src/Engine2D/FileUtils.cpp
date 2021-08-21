//
// Created by coder2k on 17.08.2021.
//

#include "FileUtils.hpp"

#include "pch.hpp"

namespace c2k::FileUtils {

    std::string readTextFile(const std::filesystem::path& path) noexcept {
        std::ifstream inputFileStream{ path };
        return std::string{ std::istreambuf_iterator<char>{ inputFileStream }, {} };
    }

}// namespace c2k::FileUtils