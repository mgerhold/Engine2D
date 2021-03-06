//
// Created by coder2k on 18.10.2021.
//

#pragma once

#include <JSON/JSON.hpp>
#include <GUID.hpp>
#include <filesystem>

namespace c2k::Viped {

    struct VipedProject {
        std::filesystem::path assetListFilename;
        GUID particleSystemGUID;
    };

    C2K_JSON_DEFINE_TYPE(VipedProject, assetListFilename, particleSystemGUID);

}// namespace c2k::Viped