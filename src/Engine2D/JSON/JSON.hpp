//
// Created by coder2k on 27.09.2021.
//

#pragma once

namespace c2k::JSON {

    struct JSONString {
        std::string value;

        [[nodiscard]] bool operator==(const JSONString&) const = default;
        [[nodiscard]] bool operator!=(const JSONString&) const = default;
    };

    struct JSONNumber {
        double value;

        [[nodiscard]] bool operator==(const JSONNumber&) const = default;
        [[nodiscard]] bool operator!=(const JSONNumber&) const = default;
    };

    struct JSONTrue {
        [[nodiscard]] bool operator==(const JSONTrue&) const = default;
        [[nodiscard]] bool operator!=(const JSONTrue&) const = default;
    };

    struct JSONFalse {
        [[nodiscard]] bool operator==(const JSONFalse&) const = default;
        [[nodiscard]] bool operator!=(const JSONFalse&) const = default;
    };

    struct JSONNull {
        [[nodiscard]] bool operator==(const JSONNull&) const = default;
        [[nodiscard]] bool operator!=(const JSONNull&) const = default;
    };

    class JSONValue;

    struct JSONArray {
        JSONArray() = default;
        JSONArray(std::initializer_list<JSONValue> init) noexcept;

        std::vector<std::shared_ptr<JSONValue>> values;

        [[nodiscard]] bool operator==(const JSONArray& other) const;
        [[nodiscard]] bool operator!=(const JSONArray& other) const;
    };

    struct JSONObject {
        JSONObject() = default;
        JSONObject(std::initializer_list<std::pair<JSONString, JSONValue>> init) noexcept;

        std::vector<std::pair<JSONString, std::shared_ptr<JSONValue>>> pairs;

        [[nodiscard]] bool operator==(const JSONObject& other) const;
        [[nodiscard]] bool operator!=(const JSONObject& other) const;
    };

    class JSONValue final {
    private:
        using JSONVariant = std::variant<JSONString, JSONNumber, JSONObject, JSONArray, JSONTrue, JSONFalse, JSONNull>;

    public:
        template<typename T>
        JSONValue(T value) noexcept : mData{ std::make_unique<JSONVariant>(std::move(value)) } { }

        [[nodiscard]] bool isString() const noexcept;
        [[nodiscard]] bool isNumber() const noexcept;
        [[nodiscard]] bool isObject() const noexcept;
        [[nodiscard]] bool isArray() const noexcept;
        [[nodiscard]] bool isTrue() const noexcept;
        [[nodiscard]] bool isFalse() const noexcept;
        [[nodiscard]] bool isNull() const noexcept;

        [[nodiscard]] tl::expected<JSONString, std::string> asString() const noexcept;
        [[nodiscard]] tl::expected<JSONNumber, std::string> asNumber() const noexcept;
        [[nodiscard]] tl::expected<JSONObject, std::string> asObject() const noexcept;
        [[nodiscard]] tl::expected<JSONArray, std::string> asArray() const noexcept;

        [[nodiscard]] bool operator==(const JSONValue& other) const;
        [[nodiscard]] bool operator!=(const JSONValue& other) const;

    private:
        template<typename T>
        [[nodiscard]] bool is() const noexcept {
            return holds_alternative<T>(*mData);
        }

        template<typename T>
        [[nodiscard]] tl::expected<T, std::string> as() const noexcept {
            if (!is<T>()) {
                return tl::unexpected(fmt::format("bad JSON value access"));
            }
            return get<T>(*mData);
        }

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

}// namespace c2k::JSON
