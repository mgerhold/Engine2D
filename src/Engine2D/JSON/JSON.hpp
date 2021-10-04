//
// Created by coder2k on 27.09.2021.
//

#pragma once

#include "Parsers.hpp"

namespace c2k::JSON {

    enum class DeserializationError {
        UnableToDeserialize,
        KeyNotFound,
        StringExpected,
        DoubleExpected,
        FloatExpected,
        IntExpected,
        BoolExpected,
        PathExpected,
        VectorExpected,
    };

    using DeserializationResult = tl::expected<std::monostate, DeserializationError>;

    namespace Implementation_ {

        template<typename T>
        void toJSON(JSONValue& json, const std::vector<T>& vector) noexcept {
            JSONArray result;
            for (const auto& value : vector) {
                result.values.emplace_back(std::make_shared<JSONValue>(value));
            }
            json = JSONValue{ result };
        }

        inline void toJSON(JSONValue& json, const std::filesystem::path& path) noexcept {
            json = JSONValue{ path.string() };
        }

        [[nodiscard]] inline DeserializationResult fromJSON(const JSONValue& json, std::string& out) noexcept {
            const auto result = json.asString();
            if (!result) {
                return tl::unexpected{ DeserializationError::StringExpected };
            }
            out = result.value();
            return std::monostate{};
        }

        [[nodiscard]] inline DeserializationResult fromJSON(const JSONValue& json, double& out) noexcept {
            const auto result = json.asNumber();
            if (!result) {
                return tl::unexpected{ DeserializationError::DoubleExpected };
            }
            out = result.value();
            return std::monostate{};
        }

        [[nodiscard]] inline DeserializationResult fromJSON(const JSONValue& json, float& out) noexcept {
            const auto result = json.asNumber();
            if (!result) {
                return tl::unexpected{ DeserializationError::FloatExpected };
            }
            out = gsl::narrow_cast<float>(result.value());
            return std::monostate{};
        }

        [[nodiscard]] inline DeserializationResult fromJSON(const JSONValue& json, int& out) noexcept {
            const auto result = json.asNumber();
            if (!result) {
                return tl::unexpected{ DeserializationError::IntExpected };
            }
            out = gsl::narrow_cast<int>(result.value());
            return std::monostate{};
        }

        [[nodiscard]] inline DeserializationResult fromJSON(const JSONValue& json, bool& out) noexcept {
            const auto result = json.asBool();
            if (!result) {
                return tl::unexpected{ DeserializationError::BoolExpected };
            }
            out = result.value();
            return std::monostate{};
        }

        template<typename T>
        [[nodiscard]] DeserializationResult fromJSON(const JSONValue& json, std::vector<T>& out) noexcept {
            if (!json.isArray()) {
                return tl::unexpected{ DeserializationError::VectorExpected };
            }
            const auto array = json.asArray().value();
            std::vector<T> result;
            for (const auto& value : array.values) {
                T buffer;
                const auto convertedExpected = fromJSON(*value, buffer);
                if (!convertedExpected) {
                    return tl::unexpected(convertedExpected.error());
                }
                result.emplace_back(buffer);
            }
            out = std::move(result);
            return std::monostate{};
        }

        [[nodiscard]] inline DeserializationResult fromJSON(const JSONValue& json,
                                                            std::filesystem::path& out) noexcept {
            const auto result = json.asString();
            if (!result) {
                return tl::unexpected{ DeserializationError::PathExpected };
            }
            out = std::filesystem::path{ result.value() };
            return std::monostate{};
        }

    }// namespace Implementation_

    using Value = Implementation_::JSONValue;
    using Array = Implementation_::JSONArray;
    using Object = Implementation_::JSONObject;

    [[nodiscard]] tl::expected<Value, std::string> fromString(const std::string& input) noexcept;
    [[nodiscard]] tl::expected<Value, std::string> fromFile(const std::filesystem::path& filename) noexcept;
    [[nodiscard]] tl::expected<Value, std::string> operator"" _json(const char* input, std::size_t) noexcept;

    template<typename T>
    [[nodiscard]] auto as(const Value& value) noexcept {
        return value.template as<T>();
    }

#include "MacroDefinitions.hpp"

}// namespace c2k::JSON