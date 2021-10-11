//
// Created by coder2k on 02.10.2021.
//

#pragma once

#include <variant>
#include <tl/expected.hpp>
#include <spdlog/spdlog.h>
#include <gsl/gsl>
#include <filesystem>
#include <type_traits>
#include <optional>

namespace c2k::JSON::Implementation_ {

    struct JSONString {
        JSONString(const char* value) : value{ value } { }
        JSONString(std::string value) : value{ std::move(value) } { }
        std::string value;

        [[nodiscard]] bool operator==(const JSONString&) const = default;
    };

    struct JSONNumber {
        double value;

        [[nodiscard]] bool operator==(const JSONNumber&) const = default;
    };

    struct JSONTrue {
        [[nodiscard]] bool operator==(const JSONTrue&) const = default;
    };

    struct JSONFalse {
        [[nodiscard]] bool operator==(const JSONFalse&) const = default;
    };

    struct JSONNull {
        [[nodiscard]] bool operator==(const JSONNull&) const = default;
    };

    class JSONValue;

    struct JSONArray {
        JSONArray() = default;
        explicit JSONArray(std::initializer_list<JSONValue> init) noexcept;

        std::vector<std::shared_ptr<JSONValue>> values;

        [[nodiscard]] bool operator==(const JSONArray& other) const;
    };

    struct JSONObject {
        JSONObject() = default;
        explicit JSONObject(std::initializer_list<std::pair<JSONString, JSONValue>> init) noexcept;

        std::vector<std::pair<JSONString, std::shared_ptr<JSONValue>>> pairs;

        [[nodiscard]] bool operator==(const JSONObject& other) const;
    };

    struct ConversionError { };

    class JSONValue final {
    private:
        using JSONVariant = std::
                variant<std::monostate, JSONString, JSONNumber, JSONObject, JSONArray, JSONTrue, JSONFalse, JSONNull>;

    public:
        JSONValue() noexcept : mData{ JSONNull{} } { }

        template<typename T>
        explicit JSONValue(const T& data) noexcept {
            toJSON(*this, data);
        }

        JSONValue(double number) noexcept : mData{ JSONVariant{ JSONNumber{ number } } } { }
        JSONValue(float number) noexcept : mData{ JSONVariant{ JSONNumber{ number } } } { }
        JSONValue(int number) noexcept : mData{ JSONVariant{ JSONNumber{ gsl::narrow_cast<double>(number) } } } { }

        JSONValue(const char* string) noexcept : mData{ JSONVariant{ JSONString{ string } } } { }

        JSONValue(std::string string) noexcept : mData{ JSONVariant{ JSONString{ std::move(string) } } } { }

        JSONValue(JSONObject object) noexcept : mData{ JSONVariant{ std::move(object) } } { }

        JSONValue(std::initializer_list<std::pair<JSONString, JSONValue>> init) noexcept
            : mData{ JSONVariant{ JSONObject{ std::move(init) } } } { }

        JSONValue(bool boolVal) noexcept : mData{ boolVal ? JSONVariant{ JSONTrue{} } : JSONVariant{ JSONFalse{} } } { }

        JSONValue(std::nullptr_t) noexcept : mData{ JSONVariant{ JSONNull{} } } { }

        JSONValue(JSONArray value) noexcept : mData{ JSONVariant{ std::move(value) } } { }

        explicit JSONValue(JSONString value) noexcept : mData{ std::move(value) } { }
        explicit JSONValue(JSONNumber value) noexcept : mData{ std::move(value) } { }
        explicit JSONValue(JSONTrue value) noexcept : mData{ std::move(value) } { }
        explicit JSONValue(JSONFalse value) noexcept : mData{ std::move(value) } { }
        explicit JSONValue(JSONNull value) noexcept : mData{ std::move(value) } { }

        [[nodiscard]] bool isString() const noexcept;
        [[nodiscard]] bool isNumber() const noexcept;
        [[nodiscard]] bool isObject() const noexcept;
        [[nodiscard]] bool isArray() const noexcept;
        [[nodiscard]] bool isTrue() const noexcept;
        [[nodiscard]] bool isFalse() const noexcept;
        [[nodiscard]] bool isNull() const noexcept;
        [[nodiscard]] bool isBool() const noexcept;
        [[nodiscard]] bool isEmpty() const noexcept;

        [[nodiscard]] tl::expected<std::string, std::string> asString() const noexcept;
        [[nodiscard]] tl::expected<double, std::string> asNumber() const noexcept;
        [[nodiscard]] tl::expected<JSONObject, std::string> asObject() const noexcept;
        [[nodiscard]] tl::expected<JSONArray, std::string> asArray() const noexcept;
        [[nodiscard]] tl::expected<bool, std::string> asBool() const noexcept;

        [[nodiscard]] bool operator==(const JSONValue& other) const;
        [[nodiscard]] std::string dump(const std::string& indentationStep = "  ") const noexcept;
        tl::expected<std::monostate, std::string> dumpToFile(const std::filesystem::path& filename) const noexcept;

        [[nodiscard]] tl::expected<JSONValue, std::string> at(const std::string& key) const noexcept;
        [[nodiscard]] bool containsKey(const std::string& key) const noexcept;

        template<typename T>
        [[nodiscard]] tl::expected<T, std::string> as(std::type_identity<T>) const noexcept {
            T result{};
            const auto success = fromJSON(*this, result);
            if (!success) {
                return tl::unexpected{ fmt::format("Unable to deserialize from JSON value") };
            }
            return result;
        }

        template<typename T>
        [[nodiscard]] tl::expected<std::optional<T>, std::string> as(
                std::type_identity<std::optional<T>>) const noexcept {
            T result{};
            const auto success = fromJSON(*this, result);
            if (!success) {
                return std::optional<T>{};
            }
            return std::optional<T>{ result };
        }

    private:
        template<typename T>
        [[nodiscard]] bool is() const noexcept {
            return holds_alternative<T>(mData);
        }

        template<typename T>
        [[nodiscard]] tl::expected<T, std::string> retrieveAs() const noexcept {
            if (!is<T>()) {
                return tl::unexpected(fmt::format("bad JSON value access"));
            }
            return get<T>(mData);
        }

        [[nodiscard]] std::string dumpImplementation(const std::string& indentationStep,
                                                     std::uint32_t baseIndentation = 0U) const noexcept;

    private:
        JSONVariant mData;
    };

    using ParsedValue = std::vector<std::variant<char,
                                                 std::string,
                                                 JSONString,
                                                 JSONNumber,
                                                 JSONTrue,
                                                 JSONFalse,
                                                 JSONNull,
                                                 JSONValue,
                                                 JSONArray,
                                                 JSONObject>>;
    using InputString = std::string_view;
    enum class ErrorType {
        InvalidSyntax,
        CharExpected,
        InvalidChar,
        InvalidString,
        DigitExpected,
        PositiveDigitExpected,
        ControlCharExpected,
        NumberOutOfRange,
        ValueExpected,
        TrueExpected,
        FalseExpected,
        NullExpected,
    };
    struct ErrorDescription {
        std::size_t remainingInputLength;
        ErrorType type;
    };
    using ResultPair = std::pair<ParsedValue, InputString>;
    using Result = tl::expected<ResultPair, ErrorDescription>;
    using Parser = std::function<Result(InputString)>;

    struct ParseJSONValueLambda {
        Result operator()(InputString input) const;
    };

    [[nodiscard]] inline ParseJSONValueLambda parseJSONValue() noexcept {
        return ParseJSONValueLambda{};
    }

}// namespace c2k::JSON::Implementation_
