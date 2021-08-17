//
// Created by coder2k on 14.08.2021.
//

#pragma once

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <fmt/format.h>
#include <algorithm>
#include <string>
#include <cassert>
#include <cstdint>

struct GUID : public boost::uuids::uuid {

    [[nodiscard]] static GUID create() noexcept {
        return fromGenerator<boost::uuids::random_generator>();
    }

    [[nodiscard]] static GUID fromString(const std::string& string) noexcept {
        return fromGenerator<boost::uuids::string_generator>(string);
    }

    [[nodiscard]] std::string string() const noexcept {
        return boost::lexical_cast<std::string>(boost::uuids::uuid{ *this });
    }

private:
    GUID() { }

    template<typename Generator, typename... Args>
    [[nodiscard]] static GUID fromGenerator(Args... args) noexcept {
        using namespace boost::uuids;
        static Generator gen;
        uuid id = gen(args...);
        GUID result;
        std::copy(std::begin(id.data), std::end(id.data), std::begin(result.data));
        return result;
    }
};

namespace std {
    template<>
    struct hash<GUID> {
        size_t operator()(const GUID& guid) const {
            return boost::hash<GUID>()(guid);
        }
    };
}// namespace std

template<>
struct fmt::formatter<GUID> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin();
        return it;
    }

    template<typename FormatContext>
    auto format(const GUID& guid, FormatContext& ctx) -> decltype(ctx.out()) {
        return format_to(ctx.out(), "{}", boost::lexical_cast<std::string>(guid));
    }
};
