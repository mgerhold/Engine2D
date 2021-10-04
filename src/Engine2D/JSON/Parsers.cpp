//
// Created by coder2k on 02.10.2021.
//

#include "Parsers.hpp"

namespace c2k::JSON::Implementation_ {

    template<typename T>
    concept ParserConcept = requires(T t, const InputString input) {
        { t(input) } -> std::same_as<Result>;
    };

    [[nodiscard]] inline auto parseAnyChar() noexcept {
        return [](const InputString input) -> Result {
            if (input.empty()) {
                return tl::unexpected{ ErrorDescription{ .remainingInputLength{ input.length() },
                                                         .type{ ErrorType::CharExpected } } };
            }
            return ResultPair({ input.front() }, input.substr(1));
        };
    }

    [[nodiscard]] inline auto parseChar(char c) noexcept {
        return [c](const InputString input) -> Result {
            if (input.empty() || input.front() != c) {
                return tl::unexpected{ ErrorDescription{ .remainingInputLength{ input.length() },
                                                         .type{ ErrorType::InvalidChar } } };
            }
            return ResultPair{ { input.front() }, input.substr(1) };
        };
    }

    [[nodiscard]] inline auto operator"" _c(char c) noexcept {
        return parseChar(c);
    }

    [[nodiscard]] inline auto parseDigit() noexcept {
        return [](const InputString input) -> Result {
            if (input.empty() || !std::isdigit(input.front())) {
                return tl::unexpected{ ErrorDescription{ .remainingInputLength{ input.length() },
                                                         .type{ ErrorType::DigitExpected } } };
            }
            return ResultPair{ { input.front() }, input.substr(1) };
        };
    }

    [[nodiscard]] inline auto parsePositiveDigit() noexcept {
        return [](const InputString input) -> Result {
            if (input.empty() || input.front() == '0' || !std::isdigit(input.front())) {
                return tl::unexpected{ ErrorDescription{ .remainingInputLength{ input.length() },
                                                         .type{ ErrorType::DigitExpected } } };
            }
            return ResultPair{ { input.front() }, input.substr(1) };
        };
    }

    [[nodiscard]] auto parserMap(auto parser, auto f) noexcept {
        return [parser = std::move(parser), f = std::move(f)](const InputString input) -> Result {
            return parser(input).and_then([&](Result&& result) -> Result { return f(std::move(result)); });
        };
    }

    [[nodiscard]] auto parserMapValue(auto parser, ParsedValue value) noexcept {
        return parserMap(std::move(parser), [value = std::move(value)](Result&& result) -> Result {
            return ResultPair{ value, result->second };
        });
    }

    [[nodiscard]] auto parseCharVecToString(auto parser) noexcept {
        return parserMap(std::move(parser), [](Result&& result) -> Result {
            std::string string;
            string.reserve(result->first.size());
            for (const auto& variant : result->first) {
                string += get<char>(variant);
            }
            return ResultPair{ { std::move(string) }, result->second };
        });
    }

    [[nodiscard]] inline auto parseString(std::string_view string) noexcept {
        return [=](const InputString input) -> Result {
            if (input.length() < string.length() || !input.starts_with(string)) {
                return tl::unexpected{ ErrorDescription{ .remainingInputLength{ input.length() },
                                                         .type{ ErrorType::InvalidString } } };
            }
            return ResultPair{ { std::string{ string } }, input.substr(string.length()) };
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
            return ResultPair{ { std::move(values) }, input };
        };
    }

    [[nodiscard]] auto parseAndDrop(auto parser) noexcept {
        return parserMapValue(std::move(parser), ParsedValue{});
    }

    [[nodiscard]] inline auto parseControlCharacter() noexcept {
        return [](const InputString input) -> Result {
            if (input.empty() || !std::iscntrl(input.front())) {
                return tl::unexpected{ ErrorDescription{ .remainingInputLength{ input.length() },
                                                         .type{ ErrorType::ControlCharExpected } } };
            }
            return ResultPair{ { input.front() }, input.substr(1) };
        };
    }

    [[nodiscard]] auto parserMapStringToJSONString(auto parser) noexcept {
        return parserMap(std::move(parser), [](Result&& result) -> Result {
            return ResultPair{ { JSONString{ std::move(get<std::string>(result->first.front())) } }, result->second };
        });
    }

    [[nodiscard]] inline auto operator+(ParserConcept auto lhs, ParserConcept auto rhs) noexcept {
        return [lhs = std::move(lhs), rhs = std::move(rhs)](const InputString input) -> Result {
            auto firstResult = lhs(input);// not const to allow implicit move on return
            if (!firstResult) {
                return firstResult;
            }
            auto secondResult = rhs(firstResult->second);// not const to allow implicit move on return
            if (!secondResult) {
                return secondResult;
            }
            auto concatenated = std::move(firstResult->first);
            concatenated.reserve(concatenated.size() + secondResult->first.size());
            for (auto it = secondResult->first.begin(); it != secondResult->first.end(); ++it) {
                concatenated.emplace_back(std::move(*it));
            }
            return ResultPair{ { std::move(concatenated) }, secondResult->second };
        };
    }

    template<typename... Parsers>
    [[nodiscard]] auto parserOr(Parsers... parsers) noexcept {
        static_assert(sizeof...(parsers) >= 2);
        return [... parsers = std::move(parsers)](InputString input) -> Result {
            Result result;
            ([&]() -> bool {
                result = parsers(input);
                return static_cast<bool>(result);
            }() || ...);
            return std::move(result);
        };
    }

    /*[[nodiscard]] inline auto operator||(ParserConcept auto lhs, ParserConcept auto rhs) noexcept {
        return [lhs = std::move(lhs), rhs = std::move(rhs)](const InputString input) -> Result {
            auto firstResult = lhs(input);// not const to allow implicit move on return
            if (firstResult) {
                return firstResult;
            }
            auto secondResult = rhs(input);// not const to allow implicit move on return
            if (secondResult) {
                return secondResult;
            }
            if (!firstResult) {
                return firstResult;
            }
            return secondResult;
        };
    }*/

    [[nodiscard]] inline auto operator>>(ParserConcept auto parser, ParsedValue value) noexcept {
        return parserMapValue(std::move(parser), std::move(value));
    }

    [[nodiscard]] inline auto operator"" _val(char c) noexcept {
        return ParsedValue{ { c } };
    }

    [[nodiscard]] inline auto operator!(ParserConcept auto parser) noexcept {
        return [parser = std::move(parser)](const InputString input) -> Result {
            auto result = parser(input);
            if (result) {
                return tl::unexpected{ ErrorDescription{ .remainingInputLength{ input.length() },
                                                         .type{ ErrorType::InvalidSyntax } } };
            }
            return ResultPair{ {}, input };
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
                                    (('\\'_c + '"'_c) >> '"'_val),
                                    (('\\'_c + '\\'_c) >> '\\'_val),
                                    (('\\'_c + '/'_c) >> '/'_val),
                                    (('\\'_c + 'b'_c) >> '\b'_val),
                                    (('\\'_c + 'f'_c) >> '\f'_val),
                                    (('\\'_c + 'n'_c) >> '\n'_val),
                                    (('\\'_c + 'r'_c) >> '\r'_val),
                                    (('\\'_c + 'r'_c) >> '\r'_val),
                                    // TODO: \u and 4 hex digits
                                    (!parserOr(parseControlCharacter(), '\\'_c, '"'_c)) + parseAnyChar()
                                )
                            ) +
                            parseAndDrop('"'_c)
                        )
                    )(input);
            // clang-format on
        };
    }

    [[nodiscard]] auto parseOptionally(auto parser) noexcept {
        return [parser = std::move(parser)](const InputString input) -> Result {
            const auto result = parser(input);
            if (result) {
                return result;
            }
            return ResultPair{ {}, input };
        };
    }

    [[nodiscard]] auto parserMapStringToJSONNumber(auto parser) noexcept {
        return parserMap(std::move(parser), [](Result&& result) -> Result {
            const double number = std::strtod(get<std::string>(result->first.front()).c_str(), nullptr);
            if (std::isinf(number) || std::isnan(number)) {
                return tl::unexpected{ ErrorDescription{ .remainingInputLength{ result->second.length() },
                                                         .type{ ErrorType::NumberOutOfRange } } };
            }
            return ResultPair{ { JSONNumber{ .value{ number } } }, result->second };
        });
    }

    [[nodiscard]] auto parseOneOrMore(auto parser) noexcept {
        return [parser = std::move(parser)](const InputString input) -> Result {
            auto result = parser(input);
            if (!result) {
                return result;
            }
            return parseZeroOrMore(parser)(input);
        };
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
        return parseAndDrop(parseZeroOrMore(parserOr(' '_c, '\n'_c, '\r'_c, '\t'_c)));
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
        return [](const InputString input) -> Result {
            // clang-format off
            auto result = (
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
                array.values.emplace_back(std::make_shared<JSONValue>(std::move(get<JSONValue>(value))));
            }
            return ResultPair{ { std::move(array) }, result->second };
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
            object.pairs.reserve(elementCount);
            for (std::size_t i = 0; i < elementCount; i += 2) {
                object.pairs.emplace_back(
                        std::pair(std::move(get<JSONString>(result->first[i])),
                                  std::make_shared<JSONValue>(std::move(get<JSONValue>(result->first[i + 1])))));
            }
            return ResultPair{ { std::move(object) }, result->second };
        };
    }

    Result ParseJSONValueLambda::operator()(const InputString input) const {
        // clang-format off
        const auto result = (parseWhitespaceAndDrop() +
                                parserOr(
                                    parseJSONString(),
                                    parseJSONNumber(),
                                    parseJSONObject(),
                                    parseJSONArray(),
                                    parseJSONTrue(),
                                    parseJSONFalse(),
                                    parseJSONNull()
                                ) +
                                parseWhitespaceAndDrop())(input);
        // clang-format on
        if (!result) {
            return result;
        }
        assert(result->first.size() == 1);
        if (holds_alternative<JSONString>(result->first.front())) {
            return ResultPair{ { JSONValue{ std::move(get<JSONString>(result->first.front())) } }, result->second };
        }
        if (holds_alternative<JSONNumber>(result->first.front())) {
            return ResultPair{ { JSONValue{ std::move(get<JSONNumber>(result->first.front())) } }, result->second };
        }
        if (holds_alternative<JSONObject>(result->first.front())) {
            return ResultPair{ { JSONValue{ std::move(get<JSONObject>(result->first.front())) } }, result->second };
        }
        if (holds_alternative<JSONArray>(result->first.front())) {
            return ResultPair{ { JSONValue{ std::move(get<JSONArray>(result->first.front())) } }, result->second };
        }
        if (holds_alternative<JSONTrue>(result->first.front())) {
            return ResultPair{ { JSONValue{ std::move(get<JSONTrue>(result->first.front())) } }, result->second };
        }
        if (holds_alternative<JSONFalse>(result->first.front())) {
            return ResultPair{ { JSONValue{ std::move(get<JSONFalse>(result->first.front())) } }, result->second };
        }
        if (holds_alternative<JSONNull>(result->first.front())) {
            return ResultPair{ { JSONValue{ std::move(get<JSONNull>(result->first.front())) } }, result->second };
        }
        return tl::unexpected{ ErrorDescription{ .remainingInputLength{ input.length() },
                                                 .type{ ErrorType::ValueExpected } } };
    }

}// namespace c2k::JSON::Implementation_
