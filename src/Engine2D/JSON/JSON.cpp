//
// Created by coder2k on 27.09.2021.
//

#include "JSON.hpp"
#include "../FileUtils/FileUtils.hpp"

namespace c2k::JSON {
    namespace Implementation_ {

        JSONArray::JSONArray(std::initializer_list<JSONValue> init) noexcept {
            for (auto&& value : init) {
                values.emplace_back(std::make_shared<JSONValue>(value));
            }
        }

        bool JSONArray::operator==(const JSONArray& other) const {
            if (values.size() != other.values.size()) {
                return false;
            }
            for (std::size_t i = 0; i < values.size(); ++i) {
                if (!values[i] || !other.values[i] || *values[i] != *other.values[i]) {
                    return false;
                }
            }
            return true;
        }

        JSONObject::JSONObject(std::initializer_list<std::pair<JSONString, JSONValue>> init) noexcept {
            for (auto&& [key, value] : init) {
                pairs.emplace_back(std::pair(key, std::make_shared<JSONValue>(value)));
            }
        }

        bool JSONObject::operator==(const JSONObject& other) const {
            if (pairs.size() != other.pairs.size()) {
                return false;
            }
            for (std::size_t i = 0; i < pairs.size(); ++i) {
                const auto& [key0, value0] = pairs[i];
                const auto& [key1, value1] = other.pairs[i];
                if (key0 != key1 || !value0 || !value1 || *value0 != *value1) {
                    return false;
                }
            }
            return true;
        }

        bool JSONValue::isString() const noexcept {
            return is<JSONString>();
        }

        bool JSONValue::isNumber() const noexcept {
            return is<JSONNumber>();
        }

        bool JSONValue::isObject() const noexcept {
            return is<JSONObject>();
        }
        bool JSONValue::isArray() const noexcept {
            return is<JSONArray>();
        }

        bool JSONValue::isTrue() const noexcept {
            return is<JSONTrue>();
        }

        bool JSONValue::isFalse() const noexcept {
            return is<JSONFalse>();
        }

        bool JSONValue::isNull() const noexcept {
            return is<JSONNull>();
        }

        [[nodiscard]] bool JSONValue::isBool() const noexcept {
            return isTrue() || isFalse();
        }

        tl::expected<std::string, std::string> JSONValue::asString() const noexcept {
            return retrieveAs<JSONString>().map([](JSONString&& jsonString) { return jsonString.value; });
        }

        tl::expected<double, std::string> JSONValue::asNumber() const noexcept {
            return retrieveAs<JSONNumber>().map([](JSONNumber&& jsonNumber) { return jsonNumber.value; });
        }

        tl::expected<JSONObject, std::string> JSONValue::asObject() const noexcept {
            return retrieveAs<JSONObject>();
        }

        tl::expected<JSONArray, std::string> JSONValue::asArray() const noexcept {
            return retrieveAs<JSONArray>();
        }

        tl::expected<bool, std::string> JSONValue::asBool() const noexcept {
            if (isTrue()) {
                return true;
            }
            if (isFalse()) {
                return false;
            }
            return tl::unexpected(fmt::format("Invalid JSON value access"));
        }

        bool JSONValue::operator==(const JSONValue& other) const {
            if (!mData || !other.mData) {
                return false;
            }
            return *mData == *other.mData;
        }

        std::string JSONValue::dump(const std::string& indentationStep) const noexcept {
            return dumpImplementation(indentationStep);
        }

        std::string JSONValue::dumpImplementation(const std::string& indentationStep,
                                                  const std::uint32_t baseIndentation) const noexcept {
            const auto addMultipleTimes = [](std::string& string, const std::string& toAppend, std::uint32_t count) {
                for (std::uint32_t i = 0U; i < count; ++i) {
                    string += toAppend;
                }
            };
            std::string result{};
            if (isString()) {
                result += fmt::format("\"{}\"", asString().value());
            } else if (isNumber()) {
                result += fmt::format("{}", asNumber().value());
            } else if (isTrue()) {
                result += "true";
            } else if (isFalse()) {
                result += "false";
            } else if (isNull()) {
                result += "null";
            } else if (isArray()) {
                result += "[\n";
                addMultipleTimes(result, indentationStep, baseIndentation + 1);
                const auto array = asArray().value();
                if (!array.values.empty()) {
                    result += array.values.front()->dumpImplementation(indentationStep, baseIndentation + 1);
                }
                for (std::size_t i = 1; i < array.values.size(); ++i) {
                    result += ",\n";
                    addMultipleTimes(result, indentationStep, baseIndentation + 1);
                    result += array.values[i]->dumpImplementation(indentationStep, baseIndentation + 1);
                }
                result += '\n';
                addMultipleTimes(result, indentationStep, baseIndentation);
                result += ']';
            } else if (isObject()) {
                result += "{\n";
                addMultipleTimes(result, indentationStep, baseIndentation + 1);
                const auto object = asObject().value();
                if (!object.pairs.empty()) {
                    result += fmt::format(
                            "\"{}\": {}", object.pairs.front().first.value,
                            object.pairs.front().second->dumpImplementation(indentationStep, baseIndentation + 1));
                }
                for (std::size_t i = 1; i < object.pairs.size(); ++i) {
                    result += ",\n";
                    addMultipleTimes(result, indentationStep, baseIndentation + 1);
                    result += fmt::format(
                            "\"{}\": {}", object.pairs[i].first.value,
                            object.pairs[i].second->dumpImplementation(indentationStep, baseIndentation + 1));
                }
                result += '\n';
                addMultipleTimes(result, indentationStep, baseIndentation);
                result += '}';
            }
            return result;
        }

        tl::expected<std::monostate, std::string> JSONValue::dumpToFile(
                const std::filesystem::path& filename) const noexcept {
            return c2k::FileUtils::writeTextFile(dump(), filename);
        }

        tl::expected<JSONValue, std::string> JSONValue::at(const std::string& key) const noexcept {
            if (!isObject()) {
                return tl::unexpected(fmt::format("Unable to access this value like an object"));
            }
            const auto& object = get<JSONObject>(*mData);
            const auto findIterator = std::find_if(object.pairs.cbegin(), object.pairs.cend(),
                                                   [&key](const auto& pair) { return pair.first.value == key; });
            if (findIterator == object.pairs.cend()) {
                return tl::unexpected(fmt::format("object does not contain the specified key '{}'", key));
            }
            return *(findIterator->second);
        }

        bool JSONValue::containsKey(const std::string& key) const noexcept {
            if (!isObject()) {
                return false;
            }
            const auto& object = get<JSONObject>(*mData);
            const auto findIterator = std::find_if(object.pairs.cbegin(), object.pairs.cend(),
                                                   [&key](const auto& pair) { return pair.first.value == key; });
            return findIterator != object.pairs.cend();
        }

        Parser operator+(const Parser& lhs, const Parser& rhs) noexcept {
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

        Parser operator||(const Parser& lhs, const Parser& rhs) noexcept {
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

        Parser operator""_c(char c) noexcept {
            return parseChar(c);
        }

        Parser operator>>(const Parser& parser, const ParsedValue& value) noexcept {
            return parserMapValue(parser, value);
        }

        ParsedValue operator""_val(char c) noexcept {
            return ParsedValue{ { c } };
        }

        Parser operator!(const Parser& parser) noexcept {
            return [parser = std::move(parser)](const InputString& input) -> Result {
                auto result = parser(input);
                if (result) {
                    return tl::unexpected(
                            fmt::format("Syntax error: {}", input.substr(0, input.length() - result->second.length())));
                }
                return parseSuccess()(input);
            };
        }

        Parser parseAnyChar() noexcept {
            return [](const InputString& input) -> Result {
                if (input.empty()) {
                    return tl::unexpected(fmt::format("Invalid input: Got empty string"));
                }
                return ResultPair({ input.front() }, input.substr(1));
            };
        }

        Parser parseChar(char c) noexcept {
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

        Parser parseDigit() noexcept {
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

        Parser parsePositiveDigit() noexcept {
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

        Parser parserMap(Parser parser, std::function<Result(Result&&)> f) noexcept {
            return [=](const InputString& input) -> Result {
                return parser(input).and_then([&](Result&& result) -> Result { return f(std::move(result)); });
            };
        }

        Parser parserMapValue(Parser parser, ParsedValue value) noexcept {
            return parserMap(parser, [value = std::move(value)](Result&& result) -> Result {
                return ResultPair{ std::move(value), result->second };
            });
        }

        Parser parseCharVecToString(Parser parser) noexcept {
            return parserMap(parser, [](Result&& result) -> Result {
                std::string string;
                string.reserve(result->first.size());
                for (auto&& variant : result->first) {
                    string += get<char>(variant);
                }
                return ResultPair{ { string }, result->second };
            });
        }

        Parser parseString(std::string string) noexcept {
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

        Parser parseZeroOrMore(Parser parser) noexcept {
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

        Parser parseJSONString() noexcept {
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

        Parser parseAndDrop(Parser parser) noexcept {
            return parserMapValue(parser, ParsedValue{});
        }

        Parser parseControlCharacter() noexcept {
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

        Parser parserMapStringToJSONString(Parser parser) noexcept {
            return parserMap(std::move(parser), [](Result&& result) -> Result {
                return ResultPair{ { JSONString{ get<std::string>(result->first.front()) } }, result->second };
            });
        }

        Parser parseOptionally(Parser parser) noexcept {
            return [parser = std::move(parser)](const InputString& input) -> Result {
                const auto result = parser(input);
                if (result) {
                    return result;
                }
                return parseSuccess()(input);
            };
        }

        Parser parseSuccess() noexcept {
            return [](const InputString& input) -> Result { return ResultPair{ {}, input }; };
        }

        Parser parseJSONNumber() noexcept {
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

        Parser parserMapStringToJSONNumber(Parser parser) noexcept {
            return parserMap(std::move(parser), [](Result&& result) -> Result {
                const std::string numberString = get<std::string>(result->first.front());
                const double number = std::strtod(numberString.c_str(), nullptr);
                if (std::isinf(number) || std::isnan(number)) {
                    return tl::unexpected(fmt::format("Value out of range: {}", numberString));
                }
                return ResultPair{ { JSONNumber{ .value{ number } } }, result->second };
            });
        }

        Parser parseOneOrMore(Parser parser) noexcept {
            return parser + parseZeroOrMore(parser);
        }

        Parser parseWhitespaceAndDrop() noexcept {
            return parseAndDrop(parseZeroOrMore(' '_c || '\n'_c || '\r'_c || '\t'_c));
        }

        Parser parseJSONTrue() noexcept {
            return parseString("true") >> ParsedValue{ JSONTrue{} };
        }

        Parser parseJSONFalse() noexcept {
            return parseString("false") >> ParsedValue{ JSONFalse{} };
        }

        Parser parseJSONNull() noexcept {
            return parseString("null") >> ParsedValue{ JSONNull{} };
        }

        Parser parseJSONValue() noexcept {
            return [](const InputString& input) -> Result {
                const auto result = (parseWhitespaceAndDrop() +
                                     (parseJSONString() || parseJSONNumber() || parseJSONObject() || parseJSONArray() ||
                                      parseJSONTrue() || parseJSONFalse() || parseJSONNull()) +
                                     parseWhitespaceAndDrop())(input);
                if (!result) {
                    return result;
                }
                assert(result->first.size() == 1);
                if (holds_alternative<JSONString>(result->first.front())) {
                    return ResultPair{ { JSONValue{ get<JSONString>(result->first.front()) } }, result->second };
                }
                if (holds_alternative<JSONNumber>(result->first.front())) {
                    return ResultPair{ { JSONValue{ get<JSONNumber>(result->first.front()) } }, result->second };
                }
                if (holds_alternative<JSONObject>(result->first.front())) {
                    return ResultPair{ { JSONValue{ get<JSONObject>(result->first.front()) } }, result->second };
                }
                if (holds_alternative<JSONArray>(result->first.front())) {
                    return ResultPair{ { JSONValue{ get<JSONArray>(result->first.front()) } }, result->second };
                }
                if (holds_alternative<JSONTrue>(result->first.front())) {
                    return ResultPair{ { JSONValue{ get<JSONTrue>(result->first.front()) } }, result->second };
                }
                if (holds_alternative<JSONFalse>(result->first.front())) {
                    return ResultPair{ { JSONValue{ get<JSONFalse>(result->first.front()) } }, result->second };
                }
                if (holds_alternative<JSONNull>(result->first.front())) {
                    return ResultPair{ { JSONValue{ get<JSONNull>(result->first.front()) } }, result->second };
                }
                return tl::unexpected{ fmt::format(
                        "error while converting parsed input into JSON value (unexpected type)") };
            };
        }

        Parser parseJSONArray() noexcept {
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

        Parser parseJSONObject() noexcept {
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

    }// namespace Implementation_

    tl::expected<Value, std::string> fromString(const std::string& input) noexcept {
        const auto result = Implementation_::parseJSONValue()(input);
        if (!result) {
            return tl::unexpected(result.error());
        }
        return get<Value>(result->first.front());
    }

    tl::expected<Value, std::string> fromFile(const std::filesystem::path& filename) noexcept {
        return c2k::FileUtils::readTextFile(filename).and_then(fromString);
    }

    tl::expected<Value, std::string> operator""_json(const char* input, std::size_t) noexcept {
        return fromString(input);
    }


}// namespace c2k::JSON
