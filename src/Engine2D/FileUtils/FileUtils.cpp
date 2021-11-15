//
// Created by coder2k on 17.08.2021.
//

#include "FileUtils.hpp"
#include <spdlog/spdlog.h>

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

    tl::expected<std::filesystem::path, std::string> findFileInParent(const std::filesystem::path& parentPath,
                                                                      std::string_view filename) noexcept {
        const auto currentFile = std::filesystem::weakly_canonical(parentPath) / filename;
        if (exists(currentFile)) {
            return currentFile;
        }
        const auto nextParent = parentPath.parent_path();
        if (nextParent == parentPath) {
            return tl::unexpected{ fmt::format("Unable to find file {}, last search location was {}", filename,
                                               parentPath.string()) };
        }
        return findFileInParent(nextParent, filename);
    }

}// namespace c2k::FileUtils