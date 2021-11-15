//
// Created by coder2k on 15.11.2021.
//

#pragma once

#include "JSON/JSON.hpp"
#include <optional>
#include <string>

namespace c2k {

    inline constexpr auto engineConfigurationFileFilename{ "engine2k.config.json" };

    struct Config {
        std::optional<std::string> assetDirectory;
    };

    C2K_JSON_DEFINE_TYPE(Config, assetDirectory);

}// namespace c2k