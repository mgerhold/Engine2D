//
// Created by coder2k on 02.10.2021.
//

#include "Parsers.hpp"

namespace c2k::JSON::Implementation_ {

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

    [[nodiscard]] inline auto operator+(ParserConcept auto lhs, ParserConcept auto rhs) noexcept {
        return [lhs = std::move(lhs), rhs = std::move(rhs)](const InputString& input) -> Result {
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

    [[nodiscard]] auto parserOr(ParserConcept auto lhs, ParserConcept auto rhs) noexcept {
        return [lhs = std::move(lhs), rhs = std::move(rhs)](const InputString& input) -> Result {
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

    /*[[nodiscard]] inline auto operator||(ParserConcept auto lhs, ParserConcept auto rhs) noexcept {
        return [lhs = std::move(lhs), rhs = std::move(rhs)](const InputString& input) -> Result {
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
    }*/

    [[nodiscard]] inline auto operator>>(ParserConcept auto parser, const ParsedValue& value) noexcept {
        return parserMapValue(std::move(parser), value);
    }

    [[nodiscard]] inline auto operator"" _val(char c) noexcept {
        return ParsedValue{ { c } };
    }

    [[nodiscard]] inline auto operator!(ParserConcept auto parser) noexcept {
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
        return [](InputString input) -> Result {
            // clang-format off
            auto result = parseAndDrop('"'_c)(input);
            if (!result) {
                return result;
            }
            input = result->second;
            return parserMapStringToJSONString(
                            parseCharVecToString(
                                parseZeroOrMore(
                                    parserOr(
                                        parserOr(
                                            parserOr(
                                                parserOr(
                                                    (('\\'_c + '"'_c) >> '"'_val),
                                                    (('\\'_c + '\\'_c) >> '\\'_val)
                                                ),
                                                parserOr(
                                                    (('\\'_c + '/'_c) >> '/'_val),
                                                    (('\\'_c + 'b'_c) >> '\b'_val)
                                                )
                                            ),
                                            parserOr(
                                                parserOr(
                                                    (('\\'_c + 'f'_c) >> '\f'_val),
                                                    (('\\'_c + 'n'_c) >> '\n'_val)
                                                ),
                                                parserOr(
                                                    (('\\'_c + 'r'_c) >> '\r'_val),
                                                    (('\\'_c + 't'_c) >> '\t'_val)
                                                )
                                            )
                                        ),
                                        // TODO: \u and 4 hex digits
                                        (!(parserOr(parserOr(parseControlCharacter(), '\\'_c), '"'_c)) + parseAnyChar())
                                    )
                                ) +
                                parseAndDrop('"'_c)
                            )
                        )(input);
            // clang-format on
        };
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
                    parserOr('0'_c, (parsePositiveDigit() + parseZeroOrMore(parseDigit()))) +
                    parseOptionally('.'_c + parseOneOrMore(parseDigit())) +
                    parseOptionally(parserOr('E'_c, 'e'_c) + parseOptionally(parserOr('+'_c, '-'_c)) + parseOneOrMore(parseDigit()))
                )
            );
        // clang-format on
    }

    [[nodiscard]] inline auto parseWhitespaceAndDrop() noexcept {
        return parseAndDrop(parseZeroOrMore(parserOr(parserOr(' '_c, '\n'_c), parserOr('\r'_c, '\t'_c))));
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

    [[nodiscard]] inline auto parseJSONArray() noexcept {
        return [](const InputString& input) -> Result {
            // clang-format off
                const auto result = (
                    parseAndDrop('['_c) + (
                        parserOr(
                            (parseJSONValue() + parseZeroOrMore(parseAndDrop(','_c) + parseJSONValue())),
                            parseWhitespaceAndDrop()
                        )
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
        return [](InputString input) -> Result {
            auto result = parseAndDrop('{'_c)(input);
            if (!result) {
                return result;
            }
            input = result->second;
            // clang-format off
                result = (
                    parserOr(
                        (
                            (
                                parseWhitespaceAndDrop() +
                                parseJSONString() +
                                parseWhitespaceAndDrop()
                            ) +
                            (
                                parseAndDrop(':'_c) +
                                parseJSONValue() +
                                parseZeroOrMore(
                                    (
                                        parseAndDrop(','_c) +
                                        parseWhitespaceAndDrop() +
                                        parseJSONString()
                                    ) +
                                    (
                                        parseWhitespaceAndDrop() +
                                        parseAndDrop(':'_c) +
                                        parseJSONValue()
                                    )
                                )
                            )
                        ),
                        parseWhitespaceAndDrop()
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
                object.pairs.emplace_back(std::pair(get<JSONString>(result->first[i]),
                                                    std::make_shared<JSONValue>(get<JSONValue>(result->first[i + 1]))));
            }
            return ResultPair{ { object }, result->second };
        };
    }

    Result ParseJSONValueLambda::operator()(const InputString& input) const {
        // clang-format off
        const auto result = (parseWhitespaceAndDrop() +
                                (
                                    parserOr(
                                        parserOr(
                                            parserOr(
                                                parseJSONString(),
                                                parseJSONNumber()
                                            ),
                                            parserOr(
                                                parseJSONObject(),
                                                parseJSONArray()
                                            )
                                        ),
                                        parserOr(
                                            parserOr(
                                                parseJSONTrue(),
                                                parseJSONFalse()
                                            ),
                                            parseJSONNull()
                                        )
                                    )
                                ) +
                                parseWhitespaceAndDrop())(input);
        // clang-format on
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
        return tl::unexpected{ fmt::format("error while converting parsed input into JSON value (unexpected type)") };
    }

}// namespace c2k::JSON::Implementation_
