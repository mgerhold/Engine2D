//
// Created by coder2k on 29.08.2021.
//

#include "GUID.hpp"

namespace c2k {
    void to_json(nlohmann::json& j, const GUID& guid) {
        j = nlohmann::json{ guid.string() };
    }

    void from_json(const nlohmann::json& j, GUID& guid) {
        std::string guidString;
        j.get_to(guidString);
        guid = GUID::fromString(guidString);
    }
}// namespace c2k
