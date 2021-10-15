//
// Created by coder2k on 27.09.2021.
//

#pragma once

#include "Parsers.hpp"
#include "Color.hpp"
#include <glm/glm.hpp>
#include <gsl/gsl>
#include <filesystem>
#include <variant>
#include <array>

namespace c2k::JSON {

    template<typename T>
    constexpr bool isOptional(const T&) {
        return false;
    }

    template<typename T>
    constexpr bool isOptional(const std::optional<T>&) {
        return true;
    }

    enum class DeserializationError {
        UnableToDeserialize,
        KeyNotFound,
        StringExpected,
        DoubleExpected,
        FloatExpected,
        IntExpected,
        UnsignedIntExpected,
        BoolExpected,
        PathExpected,
        VectorExpected,
        ObjectExpected,
    };

    using DeserializationResult = tl::expected<std::monostate, DeserializationError>;

    namespace Implementation_ {

        template<typename T>
        void toJSON(JSONValue& json, const T& value) noexcept {
            json = JSONValue{ value };
        }

        template<typename T>
        void toJSON(JSONValue& json, const std::vector<T>& vector) noexcept {
            JSONArray result;
            for (const auto& value : vector) {
                result.values.emplace_back(std::make_shared<JSONValue>(value));
            }
            json = JSONValue{ result };
        }

        template<typename T>
        void toJSON(JSONValue& json, const std::optional<T>& optional) noexcept {
            json = optional ? JSONValue{ optional.value() } : JSONValue{ std::monostate{} };
        }

        template<typename... T>
        void toJSON(JSONValue& json, const std::variant<T...>& variant) noexcept {
            ([&]() -> bool {
                if (holds_alternative<T>(variant)) {
                    toJSON(json, get<T>(variant));
                    return true;
                }
                return false;
            }() || ...);
        }

        inline void toJSON(JSONValue&, const std::monostate&) noexcept { }

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

        [[nodiscard]] inline DeserializationResult fromJSON(const JSONValue& json, std::uint32_t& out) noexcept {
            const auto result = json.asNumber();
            if (!result || result.value() < 0.0) {
                return tl::unexpected{ DeserializationError::UnsignedIntExpected };
            }
            out = gsl::narrow_cast<std::uint32_t>(result.value());
            return std::monostate{};
        }

        [[nodiscard]] inline DeserializationResult fromJSON(const JSONValue& json, glm::vec2& out) noexcept {
            if (!json.isObject()) {
                return tl::unexpected{ DeserializationError::ObjectExpected };
            }
            if (!json.containsKey("x") || !json.containsKey("y")) {
                return tl::unexpected{ DeserializationError::KeyNotFound };
            }
            const auto xResult = json.at("x")->asNumber();
            const auto yResult = json.at("y")->asNumber();
            if (!xResult || !yResult) {
                return tl::unexpected{ DeserializationError::FloatExpected };
            }
            out.x = gsl::narrow_cast<float>(xResult.value());
            out.y = gsl::narrow_cast<float>(yResult.value());
            return std::monostate{};
        }

        [[nodiscard]] inline DeserializationResult fromJSON(const JSONValue& json, glm::vec3& out) noexcept {
            if (!json.isObject()) {
                return tl::unexpected{ DeserializationError::ObjectExpected };
            }
            const bool hasXYZ = (json.containsKey("x") && json.containsKey("y") && json.containsKey("z"));
            const bool hasRGB = (json.containsKey("r") && json.containsKey("g") && json.containsKey("b"));
            if (!hasXYZ && !hasRGB) {
                return tl::unexpected{ DeserializationError::KeyNotFound };
            }
            constexpr std::array<const char*, 3> XYZKeys{ "x", "y", "z" };
            constexpr std::array<const char*, 3> RGBKeys{ "r", "g", "b" };
            std::array<const char*, 3> keys = hasXYZ ? XYZKeys : RGBKeys;
            const std::array<tl::expected<double, std::string>, 3> results{ json.at(keys[0])->asNumber(),
                                                                            json.at(keys[1])->asNumber(),
                                                                            json.at(keys[2])->asNumber() };
            for (const auto& result : results) {
                if (!result) {
                    return tl::unexpected{ DeserializationError::FloatExpected };
                }
            }
            out = glm::vec3{ gsl::narrow_cast<float>(results[0].value()), gsl::narrow_cast<float>(results[1].value()),
                             gsl::narrow_cast<float>(results[2].value()) };
            return std::monostate{};
        }

        [[nodiscard]] inline DeserializationResult fromJSON(const JSONValue& json, Color& out) noexcept {
            if (!json.isObject()) {
                return tl::unexpected{ DeserializationError::ObjectExpected };
            }
            const bool hasKeys =
                    (json.containsKey("r") && json.containsKey("g") && json.containsKey("b") && json.containsKey("a"));
            if (!hasKeys) {
                return tl::unexpected{ DeserializationError::KeyNotFound };
            }
            const std::array<tl::expected<double, std::string>, 4> results{
                json.at("r")->asNumber(), json.at("g")->asNumber(), json.at("b")->asNumber(), json.at("a")->asNumber()
            };
            for (const auto& result : results) {
                if (!result) {
                    return tl::unexpected{ DeserializationError::FloatExpected };
                }
            }
            out = { gsl::narrow_cast<float>(results[0].value()), gsl::narrow_cast<float>(results[1].value()),
                    gsl::narrow_cast<float>(results[2].value()), gsl::narrow_cast<float>(results[3].value()) };
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
            result.reserve(array.values.size());
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

        template<typename T>
        [[nodiscard]] DeserializationResult fromJSON(const JSONValue& json, std::optional<T>& out) noexcept {
            T buffer{};
            auto result = fromJSON(json, buffer);
            if (!result) {
                return result;
            }
            out = buffer;
            return std::monostate{};
        }

        template<typename... T>
        [[nodiscard]] DeserializationResult fromJSON(const JSONValue& json, std::variant<T...>& out) noexcept {
            using Variant = std::variant<T...>;
            DeserializationResult result;
            bool success = ([&]() -> bool {
                [[maybe_unused]] auto _ = holds_alternative<T>(out);// <- GCC doesn't compile without
                                                                    // this because it complains
                                                                    // that there would be no unexpanded
                                                                    // parameter pack inside the fold
                                                                    // expression operand
                T buffer{};
                result = fromJSON(json, buffer);
                if (!result) {
                    return false;
                }
                out = Variant{ std::move(buffer) };
                success = true;
                return true;
            }() || ...);
            if (success) {
                return std::monostate{};
            }
            return result;
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
        return value.as(std::type_identity<T>{});
    }

#include "MacroDefinitions.hpp"

}// namespace c2k::JSON