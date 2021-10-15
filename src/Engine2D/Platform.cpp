//
// Created by coder2k on 15.10.2021.
//

#include "Platform.hpp"
#include <tinyfiledialogs/tinyfiledialogs.h>
#include <gsl/gsl>
#include <vector>

namespace c2k {

    std::optional<std::filesystem::path> openFileDialog(const std::string& title,
                                                        const std::filesystem::path& defaultPathAndFile,
                                                        const std::vector<std::string>& filterPatterns,
                                                        const std::string& singleFilterDescription,
                                                        bool allowMultipleSelects) noexcept {
#ifdef WIN32
        std::vector<std::wstring> wideFilterPatterns;
        wideFilterPatterns.reserve(filterPatterns.size());
        for (const auto& pattern : filterPatterns) {
            wideFilterPatterns.emplace_back(pattern.cbegin(), pattern.cend());
        }
        std::vector<wchar_t const*> pointers;
        pointers.reserve(wideFilterPatterns.size());
        for (const auto& widePattern : wideFilterPatterns) {
            pointers.emplace_back(widePattern.data());
        }
        const auto defaultPath = defaultPathAndFile.wstring();
        const std::wstring wideTitle{ title.cbegin(), title.cend() };
        const std::wstring wideSingleFilterDescription{ singleFilterDescription.cbegin(),
                                                        singleFilterDescription.cend() };
        const auto result =
                tinyfd_openFileDialogW(wideTitle.c_str(), defaultPath.c_str(), gsl::narrow_cast<int>(pointers.size()),
                                       pointers.data(), wideSingleFilterDescription.c_str(), allowMultipleSelects);

#else
        std::vector<char const*> pointers;
        pointers.reserve(filterPatterns.size());
        for (const auto& widePattern : filterPatterns) {
            pointers.emplace_back(widePattern.data());
        }
        const auto result =
                tinyfd_openFileDialog(title.c_str(), defaultPathAndFile.c_str(), gsl::narrow_cast<int>(pointers.size()),
                                      pointers.data(), singleFilterDescription.c_str(), allowMultipleSelects);
#endif
        if (result == nullptr) {
            return {};
        }
        return std::filesystem::path{ result };
    };
}// namespace c2k