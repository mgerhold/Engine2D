//
// Created by coder2k on 29.08.2021.
//

#include "GUID.hpp"

namespace c2k {

    void toJSON(c2k::JSON::Value& json, const GUID& val) noexcept {
        json = JSON::Value{ val.string() };
    }

    tl::expected<std::monostate, std::string> fromJSON(const c2k::JSON::Value& json, GUID& out) noexcept {
        if (!json.isString()) {
            return tl::unexpected(fmt::format("Unable to retrieve GUID from JSON (string expected)"));
        }
        out = GUID::fromString(json.asString().value());
        return std::monostate{};
    }

}// namespace c2k
