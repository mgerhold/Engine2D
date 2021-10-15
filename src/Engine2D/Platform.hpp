//
// Created by coder2k on 15.10.2021.
//

#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace c2k {

    [[nodiscard]] std::optional<std::filesystem::path> openFileDialog(const std::string& title,
                                                                      const std::filesystem::path& defaultPathAndFile,
                                                                      const std::vector<std::string>& filterPatterns,
                                                                      const std::string& singleFilterDescription,
                                                                      bool allowMultipleSelects) noexcept;

}// namespace c2k