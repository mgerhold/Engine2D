//
// Created by coder2k on 27.09.2021.
//

#pragma once

namespace c2k::JSON {

    struct JSONString {
        std::string value;
    };

    struct JSONNumber {
        double value;
    };

    using ParsedValue = std::vector<std::variant<char, std::string, JSONString, JSONNumber>>;
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

}// namespace c2k::JSON
