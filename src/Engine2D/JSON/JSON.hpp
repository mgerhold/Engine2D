//
// Created by coder2k on 27.09.2021.
//

#pragma once

namespace c2k::JSON {

    namespace Implementation_ {

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

        class JSONValue final {
        private:
            using JSONVariant =
                    std::variant<JSONString, JSONNumber, JSONObject, JSONArray, JSONTrue, JSONFalse, JSONNull>;

        public:
            template<typename T>
            JSONValue(const T& data) noexcept {
                toJSON(*this, data);
            }

            JSONValue(double number) noexcept : mData{ std::make_shared<JSONVariant>(JSONNumber{ number }) } { }
            JSONValue(float number) noexcept : mData{ std::make_shared<JSONVariant>(JSONNumber{ number }) } { }
            JSONValue(int number) noexcept
                : mData{ std::make_shared<JSONVariant>(JSONNumber{ gsl::narrow_cast<double>(number) }) } { }

            JSONValue(const char* string) noexcept : mData{ std::make_shared<JSONVariant>(JSONString{ string }) } { }

            JSONValue(std::string string) noexcept
                : mData{ std::make_shared<JSONVariant>(JSONString{ std::move(string) }) } { }

            JSONValue(JSONObject object) noexcept : mData{ std::make_shared<JSONVariant>(std::move(object)) } { }

            JSONValue(std::initializer_list<std::pair<JSONString, JSONValue>> init) noexcept
                : mData{ std::make_shared<JSONVariant>(JSONObject{ std::move(init) }) } { }

            JSONValue(bool boolVal) noexcept {
                if (boolVal) {
                    mData = std::make_shared<JSONVariant>(JSONTrue{});
                } else {
                    mData = std::make_shared<JSONVariant>(JSONFalse{});
                }
            }

            JSONValue(std::nullptr_t) noexcept : mData{ std::make_shared<JSONVariant>(JSONNull{}) } { }

            JSONValue(JSONArray value) noexcept : mData{ std::make_shared<JSONVariant>(std::move(value)) } { }

            explicit JSONValue(JSONString value) noexcept : mData{ std::make_shared<JSONVariant>(std::move(value)) } { }
            explicit JSONValue(JSONNumber value) noexcept : mData{ std::make_shared<JSONVariant>(std::move(value)) } { }
            explicit JSONValue(JSONTrue value) noexcept : mData{ std::make_shared<JSONVariant>(std::move(value)) } { }
            explicit JSONValue(JSONFalse value) noexcept : mData{ std::make_shared<JSONVariant>(std::move(value)) } { }
            explicit JSONValue(JSONNull value) noexcept : mData{ std::make_shared<JSONVariant>(std::move(value)) } { }

            [[nodiscard]] bool isString() const noexcept;
            [[nodiscard]] bool isNumber() const noexcept;
            [[nodiscard]] bool isObject() const noexcept;
            [[nodiscard]] bool isArray() const noexcept;
            [[nodiscard]] bool isTrue() const noexcept;
            [[nodiscard]] bool isFalse() const noexcept;
            [[nodiscard]] bool isNull() const noexcept;
            [[nodiscard]] bool isBool() const noexcept;

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
            [[nodiscard]] tl::expected<T, std::string> to(const JSONValue& json) const noexcept {
                return fromJSON<T>(json);
            }

            template<typename T>
            [[nodiscard]] tl::expected<T, std::string> as() const noexcept {
                T result{};
                const auto success = fromJSON(*this, result);
                if (!success) {
                    return tl::unexpected(success.error());
                }
                return result;
            }

        private:
            template<typename T>
            [[nodiscard]] bool is() const noexcept {
                return holds_alternative<T>(*mData);
            }

            template<typename T>
            [[nodiscard]] tl::expected<T, std::string> retrieveAs() const noexcept {
                if (!is<T>()) {
                    return tl::unexpected(fmt::format("bad JSON value access"));
                }
                return get<T>(*mData);
            }

            [[nodiscard]] std::string dumpImplementation(const std::string& indentationStep,
                                                         std::uint32_t baseIndentation = 0U) const noexcept;

        private:
            std::shared_ptr<JSONVariant> mData;
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
        using InputString = std::string;
        using ErrorMessage = std::string;
        using ResultPair = std::pair<ParsedValue, InputString>;
        using Result = tl::expected<ResultPair, ErrorMessage>;
        using Parser = std::function<Result(InputString)>;

        [[nodiscard]] Parser operator+(const Parser& lhs, const Parser& rhs) noexcept;
        [[nodiscard]] Parser operator||(const Parser& lhs, const Parser& rhs) noexcept;
        [[nodiscard]] Parser operator"" _c(char c) noexcept;
        [[nodiscard]] Parser operator>>(const Parser& parser, const ParsedValue& value) noexcept;
        [[nodiscard]] ParsedValue operator"" _val(char c) noexcept;
        [[nodiscard]] Parser operator!(const Parser& parser) noexcept;

        [[nodiscard]] Parser parseAnyChar() noexcept;
        [[nodiscard]] Parser parseChar(char c) noexcept;
        [[nodiscard]] Parser parseDigit() noexcept;
        [[nodiscard]] Parser parsePositiveDigit() noexcept;
        [[nodiscard]] Parser parserMap(Parser parser, std::function<Result(Result&&)> f) noexcept;
        [[nodiscard]] Parser parserMapValue(Parser parser, ParsedValue value) noexcept;
        [[nodiscard]] Parser parseCharVecToString(Parser parser) noexcept;
        [[nodiscard]] Parser parseString(std::string string) noexcept;
        [[nodiscard]] Parser parseZeroOrMore(Parser parser) noexcept;
        [[nodiscard]] Parser parseJSONString() noexcept;
        [[nodiscard]] Parser parseAndDrop(Parser parser) noexcept;
        [[nodiscard]] Parser parseControlCharacter() noexcept;
        [[nodiscard]] Parser parserMapStringToJSONString(Parser parser) noexcept;
        [[nodiscard]] Parser parseOptionally(Parser parser) noexcept;
        [[nodiscard]] Parser parseSuccess() noexcept;
        [[nodiscard]] Parser parseJSONNumber() noexcept;
        [[nodiscard]] Parser parserMapStringToJSONNumber(Parser parser) noexcept;
        [[nodiscard]] Parser parseOneOrMore(Parser parser) noexcept;
        [[nodiscard]] Parser parseWhitespaceAndDrop() noexcept;
        [[nodiscard]] Parser parseJSONTrue() noexcept;
        [[nodiscard]] Parser parseJSONFalse() noexcept;
        [[nodiscard]] Parser parseJSONNull() noexcept;
        [[nodiscard]] Parser parseJSONValue() noexcept;
        [[nodiscard]] Parser parseJSONArray() noexcept;
        [[nodiscard]] Parser parseJSONObject() noexcept;

        template<typename T>
        void toJSON(JSONValue& json, const std::vector<T>& vector) noexcept {
            JSONArray result;
            for (const auto& value : vector) {
                result.values.emplace_back(std::make_shared<JSONValue>(value));
            }
            json = JSONValue{ result };
        }

    }// namespace Implementation_

    using Value = Implementation_::JSONValue;
    using Array = Implementation_::JSONArray;
    using Object = Implementation_::JSONObject;

    [[nodiscard]] tl::expected<Value, std::string> fromString(const std::string& input) noexcept;
    [[nodiscard]] tl::expected<Value, std::string> fromFile(const std::filesystem::path& filename) noexcept;
    // TODO: rename user-defined literal after removing nlohmann::json
    [[nodiscard]] tl::expected<Value, std::string> operator"" _asjson(const char* input, std::size_t) noexcept;

    [[nodiscard]] inline tl::expected<std::monostate, std::string> fromJSON(const Value& json,
                                                                            std::string& out) noexcept {
        const auto result = json.asString();
        if (!result) {
            return tl::unexpected{ result.error() };
        }
        out = result.value();
        return std::monostate{};
    }

    [[nodiscard]] inline tl::expected<std::monostate, std::string> fromJSON(const Value& json, double& out) noexcept {
        const auto result = json.asNumber();
        if (!result) {
            return tl::unexpected{ result.error() };
        }
        out = result.value();
        return std::monostate{};
    }

    [[nodiscard]] inline tl::expected<std::monostate, std::string> fromJSON(const Value& json, int& out) noexcept {
        const auto result = json.asNumber();
        if (!result) {
            return tl::unexpected{ result.error() };
        }
        out = gsl::narrow_cast<int>(result.value());
        return std::monostate{};
    }

    [[nodiscard]] inline tl::expected<std::monostate, std::string> fromJSON(const Value& json, bool& out) noexcept {
        const auto result = json.asBool();
        if (!result) {
            return tl::unexpected{ result.error() };
        }
        out = result.value();
        return std::monostate{};
    }

    template<typename T>
    [[nodiscard]] tl::expected<std::monostate, std::string> fromJSON(const Value& json, std::vector<T>& out) noexcept {
        if (!json.isArray()) {
            return tl::unexpected(fmt::format("Unable to convert to std::vector (JSON array expected)"));
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

    template<typename T>
    [[nodiscard]] tl::expected<T, std::string> as(const Value& value) noexcept {
        return value.template as<T>();
    }

#include "MacroDefinitions.hpp"

}// namespace c2k::JSON
