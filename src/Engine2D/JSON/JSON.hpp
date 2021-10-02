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

        template<typename T>
        concept ParserConcept = requires(T t, const InputString& input) {
            { t(input) } -> std::same_as<Result>;
        };

        [[nodiscard]] inline auto parseAnyChar() noexcept {
            return [](const InputString& input) -> Result {
                if (input.empty()) {
                    return tl::unexpected(fmt::format("Invalid input: Got empty string"));
                }
                return ResultPair({ input.front() }, input.substr(1));
            };
        }

        [[nodiscard]] inline auto parseChar(char c) noexcept {
            return [c](const InputString& input) -> Result {
                return parseAnyChar()(input).and_then([c, &input](Result&& result) -> Result {
                    if (get<char>(result->first.front()) != c) {
                        return tl::unexpected(
                                fmt::format("Invalid input: Got character '{}' (expected '{}')", input.front(), c));
                    }
                    return result;
                });
            };
        }

        [[nodiscard]] inline auto operator"" _c(char c) noexcept {
            return parseChar(c);
        }

        [[nodiscard]] inline auto parseDigit() noexcept {
            return [](const InputString& input) -> Result {
                return parseAnyChar()(input).and_then([&input](Result&& result) -> Result {
                    if (!std::isdigit(get<char>(result->first.front()))) {
                        return tl::unexpected(
                                fmt::format("Invalid input: Got character '{}' (expected digit)", input.front()));
                    }
                    return result;
                });
            };
        }

        [[nodiscard]] inline auto parsePositiveDigit() noexcept {
            return [](const InputString& input) -> Result {
                const auto result = parseDigit()(input);
                if (!result) {
                    return result;
                }
                if (get<char>(result->first.front()) == '0') {
                    return tl::unexpected(
                            fmt::format("Got {} (positive digit expected)", get<char>(result->first.front())));
                }
                return result;
            };
        }

        [[nodiscard]] auto parserMap(auto parser, auto f) noexcept {
            return [=](const InputString& input) -> Result {
                return parser(input).and_then([&](Result&& result) -> Result { return f(std::move(result)); });
            };
        }

        [[nodiscard]] auto parserMapValue(auto parser, ParsedValue value) noexcept {
            return parserMap(parser, [value = std::move(value)](Result&& result) -> Result {
                return ResultPair{ std::move(value), result->second };
            });
        }

        [[nodiscard]] auto parseCharVecToString(auto parser) noexcept {
            return parserMap(parser, [](Result&& result) -> Result {
                std::string string;
                string.reserve(result->first.size());
                for (auto&& variant : result->first) {
                    string += get<char>(variant);
                }
                return ResultPair{ { string }, result->second };
            });
        }

        [[nodiscard]] inline auto parseString(std::string string) noexcept {
            return [s = std::move(string)](const InputString& input) -> Result {
                if (s.empty()) {
                    return ResultPair{ {}, input };
                }
                auto result = parseChar(s.front())(input);
                if (!result) {
                    return result;
                }
                auto it = s.begin() + 1;
                while (result && it != s.end()) {
                    result = parseChar(*it)(result->second);
                    ++it;
                }
                if (!result) {
                    return result;
                }
                return ResultPair{ { s }, result->second };
            };
        }

        [[nodiscard]] auto parseZeroOrMore(auto parser) noexcept {
            return [parser = std::move(parser)](InputString input) -> Result {
                ParsedValue values;
                auto result = parser(input);
                while (result) {
                    values.insert(values.end(), result->first.begin(), result->first.end());
                    input = result->second;
                    result = parser(input);
                }
                return ResultPair{ { std::move(values) }, std::move(input) };
            };
        }

        [[nodiscard]] auto parseAndDrop(auto parser) noexcept {
            return parserMapValue(parser, ParsedValue{});
        }

        [[nodiscard]] inline auto parseControlCharacter() noexcept {
            return [](const InputString& input) -> Result {
                auto result = parseAnyChar()(input);
                if (!result) {
                    return result;
                }
                if (!std::iscntrl(get<char>(result->first.front()))) {
                    return tl::unexpected(
                            fmt::format("Expected control character, got '{}'", get<char>(result->first.front())));
                }
                return result;
            };
        }

        [[nodiscard]] auto parserMapStringToJSONString(auto parser) noexcept {
            return parserMap(std::move(parser), [](Result&& result) -> Result {
                return ResultPair{ { JSONString{ get<std::string>(result->first.front()) } }, result->second };
            });
        }

        [[nodiscard]] inline auto parseSuccess() noexcept {
            return [](const InputString& input) -> Result { return ResultPair{ {}, input }; };
        }

        [[nodiscard]] inline auto operator+(const Parser& lhs, const Parser& rhs) noexcept {
            return [=](const InputString& input) -> Result {
                auto firstResult = lhs(input);
                if (!firstResult) {
                    return firstResult;
                }
                const auto secondResult = rhs(firstResult->second);
                if (!secondResult) {
                    return secondResult;
                }
                using ranges::views::concat, ranges::to_vector, ranges::views::all;
                return ResultPair{ to_vector(concat(firstResult->first, secondResult->first)), secondResult->second };
            };
        }

        [[nodiscard]] inline auto operator||(const Parser& lhs, const Parser& rhs) noexcept {
            return [=](const InputString& input) -> Result {
                const auto firstResult = lhs(input);
                if (firstResult) {
                    return firstResult;
                }
                const auto secondResult = rhs(input);
                if (secondResult) {
                    return secondResult;
                }
                return tl::unexpected(
                        fmt::format("Unable to parse input with either of two parsers: Error was '{}' or '{}'",
                                    firstResult.error(), secondResult.error()));
            };
        }

        [[nodiscard]] inline auto operator>>(const Parser& parser, const ParsedValue& value) noexcept {
            return parserMapValue(parser, value);
        }

        [[nodiscard]] inline auto operator"" _val(char c) noexcept {
            return ParsedValue{ { c } };
        }

        [[nodiscard]] inline auto operator!(const Parser& parser) noexcept {
            return [parser = std::move(parser)](const InputString& input) -> Result {
                auto result = parser(input);
                if (result) {
                    return tl::unexpected(
                            fmt::format("Syntax error: {}", input.substr(0, input.length() - result->second.length())));
                }
                return parseSuccess()(input);
            };
        }

        [[nodiscard]] inline auto parseJSONString() noexcept {
            // clang-format off
            return parserMapStringToJSONString(
                parseCharVecToString(
                    parseAndDrop('"'_c) +
                    parseZeroOrMore(
                        (('\\'_c + '"'_c) >> '"'_val) ||
                        (('\\'_c + '\\'_c) >> '\\'_val) ||
                        (('\\'_c + '/'_c) >> '/'_val) ||
                        (('\\'_c + 'b'_c) >> '\b'_val) ||
                        (('\\'_c + 'f'_c) >> '\f'_val) ||
                        (('\\'_c + 'n'_c) >> '\n'_val) ||
                        (('\\'_c + 'r'_c) >> '\r'_val) ||
                        (('\\'_c + 't'_c) >> '\t'_val) ||
                        // TODO: \u and 4 hex digits
                        (!(parseControlCharacter() || '\\'_c || '"'_c) + parseAnyChar())
                    ) +
                    parseAndDrop('"'_c)
                )
            );
            // clang-format on
        }

        [[nodiscard]] auto parseOptionally(auto parser) noexcept {
            return [parser = std::move(parser)](const InputString& input) -> Result {
                const auto result = parser(input);
                if (result) {
                    return result;
                }
                return parseSuccess()(input);
            };
        }

        [[nodiscard]] auto parserMapStringToJSONNumber(auto parser) noexcept {
            return parserMap(std::move(parser), [](Result&& result) -> Result {
                const std::string numberString = get<std::string>(result->first.front());
                const double number = std::strtod(numberString.c_str(), nullptr);
                if (std::isinf(number) || std::isnan(number)) {
                    return tl::unexpected(fmt::format("Value out of range: {}", numberString));
                }
                return ResultPair{ { JSONNumber{ .value{ number } } }, result->second };
            });
        }

        [[nodiscard]] auto parseOneOrMore(auto parser) noexcept {
            return parser + parseZeroOrMore(parser);
        }

        [[nodiscard]] inline auto parseJSONNumber() noexcept {
            // clang-format off
            return parserMapStringToJSONNumber(
                parseCharVecToString(
                    parseOptionally('-'_c) +
                    ('0'_c || (parsePositiveDigit() + parseZeroOrMore(parseDigit()))) +
                    parseOptionally('.'_c + parseOneOrMore(parseDigit())) +
                    parseOptionally(('E'_c || 'e'_c) + parseOptionally('+'_c || '-'_c) + parseOneOrMore(parseDigit()))
                )
            );
            // clang-format on
        }

        [[nodiscard]] inline auto parseWhitespaceAndDrop() noexcept {
            return parseAndDrop(parseZeroOrMore(' '_c || '\n'_c || '\r'_c || '\t'_c));
        }

        [[nodiscard]] inline auto parseJSONTrue() noexcept {
            return parseString("true") >> ParsedValue{ JSONTrue{} };
        }

        [[nodiscard]] inline auto parseJSONFalse() noexcept {
            return parseString("false") >> ParsedValue{ JSONFalse{} };
        }

        [[nodiscard]] inline auto parseJSONNull() noexcept {
            return parseString("null") >> ParsedValue{ JSONNull{} };
        }

        [[nodiscard]] Parser parseJSONValue() noexcept;

        [[nodiscard]] inline auto parseJSONArray() noexcept {
            return [](const InputString& input) -> Result {
                // clang-format off
                const auto result = (
                    parseAndDrop('['_c) + (
                        (parseJSONValue() + parseZeroOrMore(parseAndDrop(','_c) + parseJSONValue())) ||
                        parseWhitespaceAndDrop()
                    ) +
                    parseAndDrop(']'_c)
                )(input);
                // clang-format on
                if (!result) {
                    return result;
                }
                JSONArray array;
                for (auto&& value : result->first) {
                    array.values.emplace_back(std::make_shared<JSONValue>(get<JSONValue>(value)));
                }
                return ResultPair{ { array }, result->second };
            };
        }

        [[nodiscard]] inline auto parseJSONObject() noexcept {
            return [](const InputString& input) -> Result {
                // clang-format off
                const auto result = (
                    parseAndDrop('{'_c) +
                    (
                        (
                            parseWhitespaceAndDrop() +
                            parseJSONString() +
                            parseWhitespaceAndDrop() +
                            parseAndDrop(':'_c) +
                            parseJSONValue() +
                            parseZeroOrMore(
                                parseAndDrop(','_c) +
                                parseWhitespaceAndDrop() +
                                parseJSONString() +
                                parseWhitespaceAndDrop() +
                                parseAndDrop(':'_c) +
                                parseJSONValue()
                            )
                        ) || parseWhitespaceAndDrop()
                    ) +
                    parseAndDrop('}'_c)
                )(input);
                // clang-format on
                if (!result) {
                    return result;
                }
                const std::size_t elementCount = result->first.size();
                assert(elementCount % 2 == 0 && "objects must consist of pairs");
                JSONObject object;
                for (std::size_t i = 0; i < elementCount; i += 2) {
                    object.pairs.emplace_back(
                            std::pair(get<JSONString>(result->first[i]),
                                      std::make_shared<JSONValue>(get<JSONValue>(result->first[i + 1]))));
                }
                return ResultPair{ { object }, result->second };
            };
        }

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

        [[nodiscard]] inline tl::expected<std::monostate, std::string> fromJSON(const JSONValue& json,
                                                                                std::string& out) noexcept {
            const auto result = json.asString();
            if (!result) {
                return tl::unexpected{ result.error() };
            }
            out = result.value();
            return std::monostate{};
        }

        [[nodiscard]] inline tl::expected<std::monostate, std::string> fromJSON(const JSONValue& json,
                                                                                double& out) noexcept {
            const auto result = json.asNumber();
            if (!result) {
                return tl::unexpected{ result.error() };
            }
            out = result.value();
            return std::monostate{};
        }

        [[nodiscard]] inline tl::expected<std::monostate, std::string> fromJSON(const JSONValue& json,
                                                                                float& out) noexcept {
            const auto result = json.asNumber();
            if (!result) {
                return tl::unexpected{ result.error() };
            }
            out = gsl::narrow_cast<float>(result.value());
            return std::monostate{};
        }

        [[nodiscard]] inline tl::expected<std::monostate, std::string> fromJSON(const JSONValue& json,
                                                                                int& out) noexcept {
            const auto result = json.asNumber();
            if (!result) {
                return tl::unexpected{ result.error() };
            }
            out = gsl::narrow_cast<int>(result.value());
            return std::monostate{};
        }

        [[nodiscard]] inline tl::expected<std::monostate, std::string> fromJSON(const JSONValue& json,
                                                                                bool& out) noexcept {
            const auto result = json.asBool();
            if (!result) {
                return tl::unexpected{ result.error() };
            }
            out = result.value();
            return std::monostate{};
        }

        template<typename T>
        [[nodiscard]] tl::expected<std::monostate, std::string> fromJSON(const JSONValue& json,
                                                                         std::vector<T>& out) noexcept {
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

        [[nodiscard]] inline tl::expected<std::monostate, std::string> fromJSON(const JSONValue& json,
                                                                                std::filesystem::path& out) noexcept {
            const auto result = json.asString();
            if (!result) {
                return tl::unexpected(fmt::format("Unable to convert JSON to filesystem path (string expected)"));
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
    [[nodiscard]] tl::expected<T, std::string> as(const Value& value) noexcept {
        return value.template as<T>();
    }

#include "MacroDefinitions.hpp"

}// namespace c2k::JSON