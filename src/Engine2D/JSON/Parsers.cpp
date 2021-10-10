//
// Created by coder2k on 02.10.2021.
//

#include "Parsers.hpp"

namespace c2k::JSON::Implementation_ {

    template<typename T>
    concept ParserConcept = requires(T t, const InputString input) {
        { t(input) } -> std::same_as<Result>;
    };

    [[nodiscard]] consteval inline auto parseAnyChar() noexcept {
        return [](const InputString input) -> Result {
            if (input.empty()) {
                return tl::unexpected{ ErrorDescription{ .remainingInputLength{ input.length() },
                                                         .type{ ErrorType::CharExpected } } };
            }
            return ResultPair({ input.front() }, input.substr(1));
        };
    }

    [[nodiscard]] consteval inline auto parseChar(char c) noexcept {
        return [c](const InputString input) -> Result {
            if (input.empty() || input.front() != c) {
                return tl::unexpected{ ErrorDescription{ .remainingInputLength{ input.length() },
                                                         .type{ ErrorType::InvalidChar } } };
            }
            return ResultPair{ { input.front() }, input.substr(1) };
        };
    }

    [[nodiscard]] consteval inline auto operator"" _c(char c) noexcept {
        return parseChar(c);
    }

    [[nodiscard]] consteval inline auto parseDigit() noexcept {
        return [](const InputString input) -> Result {
            if (input.empty() || !std::isdigit(input.front())) {
                return tl::unexpected{ ErrorDescription{ .remainingInputLength{ input.length() },
                                                         .type{ ErrorType::DigitExpected } } };
            }
            return ResultPair{ { input.front() }, input.substr(1) };
        };
    }

    [[nodiscard]] consteval inline auto parsePositiveDigit() noexcept {
        return [](const InputString input) -> Result {
            if (input.empty() || input.front() == '0' || !std::isdigit(input.front())) {
                return tl::unexpected{ ErrorDescription{ .remainingInputLength{ input.length() },
                                                         .type{ ErrorType::DigitExpected } } };
            }
            return ResultPair{ { input.front() }, input.substr(1) };
        };
    }


    [[nodiscard]] constexpr auto parserMapValue(ParserConcept auto parser, ParsedValue value) noexcept {
        return [parser = std::move(parser), value = std::move(value)](const InputString input) -> Result {
            const auto result = parser(input);
            if (!result) {
                return result;
            }
            return ResultPair{ value, result->second };
        };
    }

    [[nodiscard]] constexpr auto parseCharVecToString(ParserConcept auto parser) noexcept {
        return [parser = std::move(parser)](const InputString input) -> Result {
            const auto result = parser(input);
            if (!result) {
                return result;
            }
            std::string string;
            string.reserve(result->first.size());
            for (const auto& variant : result->first) {
                string += get<char>(variant);
            }
            return ResultPair{ { std::move(string) }, result->second };
        };
    }

    [[nodiscard]] consteval inline auto parseString(std::string_view string) noexcept {
        return [=](const InputString input) -> Result {
            if (input.length() < string.length() || !input.starts_with(string)) {
                return tl::unexpected{ ErrorDescription{ .remainingInputLength{ input.length() },
                                                         .type{ ErrorType::InvalidString } } };
            }
            return ResultPair{ { std::string{ string } }, input.substr(string.length()) };
        };
    }

    [[nodiscard]] constexpr auto parseZeroOrMore(ParserConcept auto parser) noexcept {
        return [parser = std::move(parser)](InputString input) -> Result {
            ParsedValue values;
            auto result = parser(input);
            while (result) {
                values.reserve(values.size() + result->first.size());
                std::move(result->first.begin(), result->first.end(), std::back_inserter(values));
                input = result->second;
                result = parser(input);
            }
            return ResultPair{ std::move(values), input };
        };
    }

    [[nodiscard]] constexpr auto parseAndDrop(ParserConcept auto parser) noexcept {
        return [parser = std::move(parser)](const InputString input) -> Result {
            const auto result = parser(input);
            if (!result) {
                return result;
            }
            return ResultPair{ {}, result->second };
        };
    }

    [[nodiscard]] consteval inline auto parseControlCharacter() noexcept {
        return [](const InputString input) -> Result {
            if (input.empty() || !std::iscntrl(input.front())) {
                return tl::unexpected{ ErrorDescription{ .remainingInputLength{ input.length() },
                                                         .type{ ErrorType::ControlCharExpected } } };
            }
            return ResultPair{ { input.front() }, input.substr(1) };
        };
    }

    [[nodiscard]] constexpr auto parserMapStringToJSONString(ParserConcept auto parser) noexcept {
        return [parser = std::move(parser)](const InputString input) -> Result {
            const auto result = parser(input);
            if (!result) {
                return result;
            }
            return ResultPair{ { JSONString{ std::move(get<std::string>(result->first.front())) } }, result->second };
        };
    }

    [[nodiscard]] constexpr auto concat(ParserConcept auto... parsers) noexcept {
        static_assert(sizeof...(parsers) >= 2);
        return [=](const InputString input) -> Result {
            Result result = ResultPair{ {}, input };
            ([&]() {
                assert(result);
                auto& remainingInput = result->second;
                auto newResult = parsers(remainingInput);
                if (!newResult) {
                    result = newResult;
                    return false;
                }
                auto& [newValues, newRemainingInput] = newResult.value();
                std::move(begin(newValues), end(newValues), std::back_inserter(result->first));
                result->second = newRemainingInput;
                return true;
            }() &&
             ...);
            return result;
        };
    }

    [[nodiscard]] constexpr auto parserOr(ParserConcept auto... parsers) noexcept {
        static_assert(sizeof...(parsers) >= 2);
        return [=](InputString input) -> Result {
            Result result;
            ([&]() -> bool {
                result = parsers(input);
                return static_cast<bool>(result);
            }() || ...);
            return result;
        };
    }

    [[nodiscard]] constexpr inline auto operator>>(ParserConcept auto parser, ParsedValue value) noexcept {
        return parserMapValue(std::move(parser), std::move(value));
    }

    [[nodiscard]] inline auto operator"" _val(char c) noexcept {
        return ParsedValue{ { c } };
    }

    [[nodiscard]] constexpr inline auto operator!(ParserConcept auto parser) noexcept {
        return [parser = std::move(parser)](const InputString input) -> Result {
            auto result = parser(input);
            if (result) {
                return tl::unexpected{ ErrorDescription{ .remainingInputLength{ input.length() },
                                                         .type{ ErrorType::InvalidSyntax } } };
            }
            return ResultPair{ {}, input };
        };
    }

    [[nodiscard]] constexpr inline auto parseJSONString() noexcept {
        return [](InputString input) -> Result {
            // clang-format off
            return parserMapStringToJSONString(
                        parseCharVecToString(
                            concat(
                                parseAndDrop('"'_c),
                                parseZeroOrMore(
                                    parserOr(
                                        concat((!parserOr(parseControlCharacter(), '\\'_c, '"'_c)), parseAnyChar()),
                                        concat('\\'_c, '"'_c) >> '"'_val,
                                        concat('\\'_c, '\\'_c) >> '\\'_val,
                                        concat('\\'_c, '/'_c) >> '/'_val,
                                        concat('\\'_c, 'b'_c) >> '\b'_val,
                                        concat('\\'_c, 'f'_c) >> '\f'_val,
                                        concat('\\'_c, 'n'_c) >> '\n'_val,
                                        concat('\\'_c, 'r'_c) >> '\r'_val,
                                        concat('\\'_c, 'r'_c) >> '\r'_val
                                        // TODO: \u and 4 hex digits
                                    )
                                ),
                                parseAndDrop('"'_c)
                            )
                        )
                    )(input);
            // clang-format on
        };
    }

    [[nodiscard]] constexpr auto parseOptionally(ParserConcept auto parser) noexcept {
        return [parser = std::move(parser)](const InputString input) -> Result {
            auto result = parser(input);
            if (result) {
                return result;
            }
            return ResultPair{ {}, input };
        };
    }

    [[nodiscard]] constexpr auto parserMapStringToJSONNumber(ParserConcept auto parser) noexcept {
        return [parser = std::move(parser)](const InputString input) -> Result {
            const auto result = parser(input);
            if (!result) {
                return result;
            }
            const double number = std::strtod(get<std::string>(result->first.front()).c_str(), nullptr);
            if (std::isinf(number) || std::isnan(number)) {
                return tl::unexpected{ ErrorDescription{ .remainingInputLength{ result->second.length() },
                                                         .type{ ErrorType::NumberOutOfRange } } };
            }
            return ResultPair{ { JSONNumber{ .value{ number } } }, result->second };
        };
    }

    [[nodiscard]] constexpr auto parseOneOrMore(ParserConcept auto parser) noexcept {
        return [parser = std::move(parser)](InputString input) -> Result {
            ParsedValue values;
            auto result = parser(input);
            if (!result) {
                return result;
            }
            while (result) {
                values.reserve(values.size() + result->first.size());
                std::move(result->first.begin(), result->first.end(), std::back_inserter(values));
                input = result->second;
                result = parser(input);
            }
            return ResultPair{ std::move(values), input };
        };
    }

    [[nodiscard]] constexpr inline auto parseJSONNumber() noexcept {
        // clang-format off
        return parserMapStringToJSONNumber(
            parseCharVecToString(
                concat(
                    parseOptionally('-'_c),
                    parserOr('0'_c, concat(parsePositiveDigit(), parseZeroOrMore(parseDigit()))),
                    parseOptionally(concat('.'_c, parseOneOrMore(parseDigit()))),
                    parseOptionally(
                        concat(
                            parserOr('E'_c, 'e'_c),
                            parseOptionally(parserOr('+'_c, '-'_c)),
                            parseOneOrMore(parseDigit())
                        )
                    )
                )
            )
        );
        // clang-format on
    }

    [[nodiscard]] consteval inline auto parseWhitespaceAndDrop() noexcept {
        return [](InputString input) -> Result {
            while (!input.empty() &&
                   (input.front() == ' ' || input.front() == '\n' || input.front() == '\r' || input.front() == '\t')) {
                input = input.substr(1);
            }
            return ResultPair{ {}, input };
        };
    }

    [[nodiscard]] consteval inline auto parseJSONTrue() noexcept {
        constexpr std::size_t tokenLength{ 4 };
        return [](const InputString input) -> Result {
            if (input.length() < tokenLength || !input.starts_with("true")) {
                return tl::unexpected{ ErrorDescription{ .remainingInputLength{ input.length() },
                                                         .type{ ErrorType::TrueExpected } } };
            }
            return ResultPair{ { JSONTrue{} }, input.substr(tokenLength) };
        };
    }

    [[nodiscard]] consteval inline auto parseJSONFalse() noexcept {
        constexpr std::size_t tokenLength{ 5 };
        return [](const InputString input) -> Result {
            if (input.length() < tokenLength || !input.starts_with("false")) {
                return tl::unexpected{ ErrorDescription{ .remainingInputLength{ input.length() },
                                                         .type{ ErrorType::FalseExpected } } };
            }
            return ResultPair{ { JSONFalse{} }, input.substr(tokenLength) };
        };
    }

    [[nodiscard]] consteval inline auto parseJSONNull() noexcept {
        constexpr std::size_t tokenLength{ 4 };
        return [](const InputString input) -> Result {
            if (input.length() < tokenLength || !input.starts_with("null")) {
                return tl::unexpected{ ErrorDescription{ .remainingInputLength{ input.length() },
                                                         .type{ ErrorType::NullExpected } } };
            }
            return ResultPair{ { JSONNull{} }, input.substr(tokenLength) };
        };
    }

    [[nodiscard]] constexpr inline auto parseJSONArray() noexcept {
        return [](const InputString input) -> Result {
            // clang-format off
            auto result = (
                concat(
                    parseAndDrop('['_c),
                    (
                        parserOr(
                            concat(
                                parseJSONValue(),
                                parseZeroOrMore(
                                    concat(
                                        parseAndDrop(','_c),
                                        parseJSONValue()
                                    )
                                )
                            ),
                            parseWhitespaceAndDrop()
                        )
                    ),
                    parseAndDrop(']'_c)
                )
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

    [[nodiscard]] constexpr inline auto parseJSONObject() noexcept {
        return [](InputString input) -> Result {
            // clang-format off
            auto result = (
                concat(
                    parseAndDrop('{'_c),
                    parserOr(
                        concat(
                            parseWhitespaceAndDrop(),
                            parseJSONString(),
                            parseWhitespaceAndDrop(),
                            parseAndDrop(':'_c),
                            parseJSONValue(),
                            parseZeroOrMore(
                                concat(
                                    parseAndDrop(','_c),
                                    parseWhitespaceAndDrop(),
                                    parseJSONString(),
                                    parseWhitespaceAndDrop(),
                                    parseAndDrop(':'_c),
                                    parseJSONValue()
                                )
                            )
                        ),
                        parseWhitespaceAndDrop()
                    ),
                    parseAndDrop('}'_c)
                )
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
        auto result = (
            concat(
                parseWhitespaceAndDrop(),
                parserOr(
                    parseJSONObject(),
                    parseJSONArray(),
                    parseJSONString(),
                    parseJSONNumber(),
                    parseJSONTrue(),
                    parseJSONFalse(),
                    parseJSONNull()
                ),
                parseWhitespaceAndDrop()
            )
        )(input);
        // clang-format on
        if (!result) {
            return result;
        }
        assert(result->first.size() == 1);
        if (holds_alternative<JSONObject>(result->first.front())) {
            return ResultPair{ { JSONValue{ std::move(get<JSONObject>(result->first.front())) } }, result->second };
        }
        if (holds_alternative<JSONArray>(result->first.front())) {
            return ResultPair{ { JSONValue{ std::move(get<JSONArray>(result->first.front())) } }, result->second };
        }
        if (holds_alternative<JSONString>(result->first.front())) {
            return ResultPair{ { JSONValue{ std::move(get<JSONString>(result->first.front())) } }, result->second };
        }
        if (holds_alternative<JSONNumber>(result->first.front())) {
            return ResultPair{ { JSONValue{ std::move(get<JSONNumber>(result->first.front())) } }, result->second };
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
