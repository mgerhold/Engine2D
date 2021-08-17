//
// Created by coder2k on 17.08.2021.
//

#include "FileUtils.hpp"

#include <fstream>
#include <iterator>

std::string FileUtils::readTextFile(const std::filesystem::path& path) noexcept {
    std::ifstream inputFileStream{ path };
    return std::string{ std::istreambuf_iterator<char>{ inputFileStream }, {} };
}
