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

    tl::expected<std::monostate, std::string> writeTextFile(const std::string& text,
                                                            const std::filesystem::path& path) noexcept {
        std::ofstream outputFileStream{ path };
        if (!outputFileStream.good()) {
            return tl::unexpected(fmt::format("Unable to open file for writing: {}", path.string()));
        }
        outputFileStream << text;
        outputFileStream.close();
        return std::monostate{};
    }

}// namespace c2k::FileUtils