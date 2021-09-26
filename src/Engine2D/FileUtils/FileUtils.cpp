//
// Created by coder2k on 17.08.2021.
//

#include "FileUtils.hpp"

namespace c2k::FileUtils {

    tl::expected<std::string, std::string> readTextFile(const std::filesystem::path& path) noexcept {
        std::ifstream inputFileStream{ path };
        if (!inputFileStream.good()) {
            return tl::unexpected<std::string>(fmt::format("Unable to open file {}", path.string()));
        }
        return std::string{ std::istreambuf_iterator<char>{ inputFileStream }, {} };
    }

}// namespace c2k::FileUtils