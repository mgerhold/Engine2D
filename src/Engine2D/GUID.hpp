//
// Created by coder2k on 14.08.2021.
//

#pragma once

#include "JSON/JSON.hpp"

namespace c2k {

    struct GUID : public boost::uuids::uuid {
        GUID() = default;

        GUID(const GUID& other) noexcept {
            std::copy(std::begin(other.data), std::end(other.data), std::begin(data));
        }

        GUID& operator=(const GUID& other) noexcept {
            std::copy(std::begin(other.data), std::end(other.data), std::begin(data));
            return *this;
        }

        [[nodiscard]] static GUID create() noexcept {
            return fromGenerator<boost::uuids::random_generator>();
        }

        [[nodiscard]] static GUID fromString(const std::string& string) noexcept {
            return fromGenerator<boost::uuids::string_generator>(string);
        }

        [[nodiscard]] std::string string() const noexcept {
            return boost::lexical_cast<std::string>(boost::uuids::uuid{ *this });
        }

        [[nodiscard]] static GUID invalid() noexcept {
            static const auto result{ create() };
            return result;
        }

    private:
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

    void toJSON(JSON::Value& json, const GUID& val) noexcept;

    [[nodiscard]] tl::expected<std::monostate, std::string> fromJSON(const JSON::Value& json, GUID& out) noexcept;

}// namespace c2k

template<>
struct fmt::formatter<c2k::GUID> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin();
        return it;
    }

    template<typename FormatContext>
    auto format(const c2k::GUID& guid, FormatContext& ctx) -> decltype(ctx.out()) {
        return format_to(ctx.out(), "{}", boost::lexical_cast<std::string>(guid));
    }
};

namespace std {

    template<>
    struct hash<c2k::GUID> {
        size_t operator()(const c2k::GUID& guid) const {
            return boost::hash<c2k::GUID>()(guid);
        }
    };

}// namespace std