//
// Created by coder2k on 14.08.2021.
//

#pragma once

#include "JSON/JSON.hpp"
#include <spdlog/spdlog.h>
#include <iterator>
#include <algorithm>
#include <string>
#include <sstream>
#include <random>
#include <cstdint>

namespace c2k {

    struct GUID {
        [[nodiscard]] bool operator==(const GUID&) const = default;

        [[nodiscard]] std::uint64_t low() const noexcept {
            return mLow;
        }

        [[nodiscard]] std::uint64_t high() const noexcept {
            return mHigh;
        }

        [[nodiscard]] static GUID create() noexcept {
            GUID result;
            result.mHigh = mDistribution(mRandomGenerator);
            result.mLow = mDistribution(mRandomGenerator);
            return result;
        }

        [[nodiscard]] static GUID fromString(std::string string) noexcept {
            string.erase(std::remove(string.begin(), string.end(), '-'), string.end());
            std::stringstream highStream;
            highStream << std::hex << string.substr(0, 16);
            std::stringstream lowStream;
            lowStream << std::hex << string.substr(16);
            GUID result;
            highStream >> result.mHigh;
            lowStream >> result.mLow;
            return result;
        }

        [[nodiscard]] std::string string() const noexcept {
            std::stringstream ss;
            ss << std::hex << std::setfill('0') << std::setw(16) << mHigh << std::setw(16) << mLow;
            auto result = ss.str();
            result.insert(8, 1, '-');
            result.insert(13, 1, '-');
            result.insert(18, 1, '-');
            result.insert(23, 1, '-');
            return result;
        }

        [[nodiscard]] static GUID invalid() noexcept {
            static const auto result{ create() };
            return result;
        }

    private:
        static inline std::random_device mRandomDevice{};
        static inline std::mt19937_64 mRandomGenerator{ mRandomDevice() };
        static inline std::uniform_int_distribution<std::uint64_t> mDistribution{};

        std::uint64_t mHigh{ 0 }, mLow{ 0 };
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
        return format_to(ctx.out(), "{}", guid.string());
    }
};

namespace std {

    template<>
    struct hash<c2k::GUID> {
        size_t operator()(const c2k::GUID& guid) const {
            return guid.low() ^ guid.high();
        }
    };

}// namespace std