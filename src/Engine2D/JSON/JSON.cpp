//
// Created by coder2k on 27.09.2021.
//

#include "JSON.hpp"

namespace c2k::JSON {

    Parser operator+(const Parser& lhs, const Parser& rhs) noexcept {
        return [=](const InputString& input) -> Result {
            auto firstResult = lhs(input);
            if (!firstResult) {
                return tl::unexpected(firstResult.error());
            }
            const auto secondResult = rhs(firstResult->second);
            if (!secondResult) {
                return tl::unexpected(secondResult.error());
            }
            using ranges::views::concat, ranges::to_vector, ranges::views::all;
            return ResultPair(to_vector(concat(firstResult->first, secondResult->first)), secondResult->second);
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
            return ResultPair{ { std::move(values) }, input };
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
            return ResultPair{ { JSONString{ .value{ get<std::string>(result->first.front()) } } }, result->second };
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
                        parseOptionally(parseChar('-')) +
                        ('0'_c || (parsePositiveDigit() + parseZeroOrMore(parseDigit()))) +
                        parseOptionally('.'_c + parseOneOrMore(parseDigit())) +
                        parseOptionally(
                            ('E'_c || 'e'_c) + parseOptionally('+'_c || '-'_c) + parseOneOrMore(parseDigit())
                        )
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

    Parser parseWhitespace() noexcept {
        return parseOptionally(' '_c || '\n'_c || '\r'_c || '\t'_c);
    }

}// namespace c2k::JSON
