//
// Created by coder2k on 27.09.2021.
//

#include <JSON/JSON.hpp>
#include <gtest/gtest.h>

TEST(PrimitiveParsers, parseChar) {
    using namespace c2k::JSON;
    const std::string input = "abc";
    auto result = parseChar('a')(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<char>(result->first.front()), 'a');
    ASSERT_EQ(result->second, "bc");

    result = parseChar('b')(input);
    ASSERT_FALSE(result);
    spdlog::info("Parser returned error message: {}", result.error());
}

TEST(PrimitiveParsers, operatorLogicalOr) {
    using namespace c2k::JSON;
    std::string input = "abc";
    auto result = (parseChar('a') || parseChar('b'))(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<char>(result->first.front()), 'a');
    ASSERT_EQ(result->second, "bc");

    input = "bac";
    result = (parseChar('a') || parseChar('b'))(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<char>(result->first.front()), 'b');
    ASSERT_EQ(result->second, "ac");

    input = "cab";
    result = (parseChar('a') || parseChar('b'))(input);
    ASSERT_FALSE(result);
    spdlog::info("Parser returned error message: {}", result.error());
}

TEST(PrimitiveParsers, operatorPlusForTwoParsers) {
    using namespace c2k::JSON;
    std::string input = "abc";
    auto result = parseCharVecToString(parseChar('a') + parseChar('b'))(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<std::string>(result->first.front()), "ab");
    ASSERT_EQ(result->second, "c");

    input = "bac";
    result = parseCharVecToString(parseChar('a') + parseChar('b'))(input);
    ASSERT_FALSE(result);
    spdlog::info("Parser returned error message: {}", result.error());
}

TEST(PrimitiveParsers, parseDigit) {
    using namespace c2k::JSON;
    std::string input = "01abc";
    auto result = parseDigit()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<char>(result->first.front()), '0');
    ASSERT_EQ(result->second, "1abc");
}

TEST(CombinedParsers, operatorPlus) {
    using namespace c2k::JSON;
    std::string input = "constexprvolatile";
    auto result = parseCharVecToString('c'_c + 'o'_c + 'n'_c + 's'_c + 't'_c + 'e'_c + 'x'_c + 'p'_c + 'r'_c)(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<std::string>(result->first.front()), "constexpr");
    ASSERT_EQ(result->second, "volatile");

    input = "Constexprvolatile";
    result = parseCharVecToString('c'_c + 'o'_c + 'n'_c + 's'_c + 't'_c + 'e'_c + 'x'_c + 'p'_c + 'r'_c)(input);
    ASSERT_FALSE(result);
    spdlog::info("Parser returned error message: {}", result.error());
}

TEST(CombinedParsers, parseString) {
    using namespace c2k::JSON;
    std::string input = "constexprvolatile";
    auto result = parseString("constexpr")(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<std::string>(result->first.front()), "constexpr");
    ASSERT_EQ(result->second, "volatile");

    input = "Constexprvolatile";
    result = parseString("constexpr")(input);
    ASSERT_FALSE(result);
    spdlog::info("Parser returned error message: {}", result.error());
}

TEST(CombinedParsers, parseZeroOrMore) {
    using namespace c2k::JSON;
    std::string input = "aaaaab";
    auto result = parseCharVecToString(parseZeroOrMore(parseChar('a')))(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<std::string>(result->first.front()), "aaaaa");
    ASSERT_EQ(result->second, "b");

    input = "aaaaab";
    result = parseCharVecToString(parseZeroOrMore(parseChar('b')))(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<std::string>(result->first.front()), "");
    ASSERT_EQ(result->second, "aaaaab");
}

TEST(CombinedParsers, parseJSONString) {
    using namespace c2k::JSON;
    std::string input = R"("My JSON String"do not match this!)";
    auto result = parseJSONString()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONString>(result->first.front()).value, "My JSON String");
    ASSERT_EQ(result->second, "do not match this!");

    input = R"("new\nline"do not match this!)";
    result = parseJSONString()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONString>(result->first.front()).value, "new\nline");
    ASSERT_EQ(result->second, "do not match this!");

    input = "\"new\nline\"do not match this!)";
    result = parseJSONString()(input);
    ASSERT_FALSE(result);
    spdlog::info("Parser returned error message: {}", result.error());
}

TEST(CombinedParsers, parseControlCharacter) {
    using namespace c2k::JSON;
    std::string input = "\1abc";
    auto result = parseControlCharacter()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<char>(result->first.front()), '\1');
    ASSERT_EQ(result->second, "abc");

    input = "abc";
    result = parseControlCharacter()(input);
    ASSERT_FALSE(result);
    spdlog::info("Parser returned error message: {}", result.error());
}

TEST(CombinedParsers, parseJSONNumber) {
    using namespace c2k::JSON;
    std::string input = "0abc";
    auto result = parseJSONNumber()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONNumber>(result->first.front()).value, std::stod("0"));
    ASSERT_EQ(result->second, "abc");

    input = "-0abc";
    result = parseJSONNumber()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONNumber>(result->first.front()).value, std::stod("-0"));
    ASSERT_EQ(result->second, "abc");

    input = "123abc";
    result = parseJSONNumber()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONNumber>(result->first.front()).value, std::stod("123"));
    ASSERT_EQ(result->second, "abc");

    input = "-123abc";
    result = parseJSONNumber()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONNumber>(result->first.front()).value, std::stod("-123"));
    ASSERT_EQ(result->second, "abc");

    input = "-0123abc";
    result = parseJSONNumber()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONNumber>(result->first.front()).value, std::stod("-0"));
    ASSERT_EQ(result->second, "123abc");

    input = "abc";
    result = parseJSONNumber()(input);
    ASSERT_FALSE(result);

    input = "-10203abc";
    result = parseJSONNumber()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONNumber>(result->first.front()).value, std::stod("-10203"));
    ASSERT_EQ(result->second, "abc");

    input = "123.abc";
    result = parseJSONNumber()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONNumber>(result->first.front()).value, std::stod("123"));
    ASSERT_EQ(result->second, ".abc");

    input = "123.0abc";
    result = parseJSONNumber()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONNumber>(result->first.front()).value, std::stod("123.0"));
    ASSERT_EQ(result->second, "abc");

    input = "-123.0123abc";
    result = parseJSONNumber()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONNumber>(result->first.front()).value, std::stod("-123.0123"));
    ASSERT_EQ(result->second, "abc");

    input = "123.0eabc";
    result = parseJSONNumber()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONNumber>(result->first.front()).value, std::stod("123.0"));
    ASSERT_EQ(result->second, "eabc");

    input = "123.0e5abc";
    result = parseJSONNumber()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONNumber>(result->first.front()).value, std::stod("123.0e5"));
    ASSERT_EQ(result->second, "abc");

    input = "123.0e558abc";
    result = parseJSONNumber()(input);
    ASSERT_FALSE(result);
    spdlog::info("Parser returned error message: {}", result.error());

    input = "123e5abc";
    result = parseJSONNumber()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONNumber>(result->first.front()).value, std::stod("123e5"));
    ASSERT_EQ(result->second, "abc");
}

TEST(CombinedParsers, parseWhitespace) {
    using namespace c2k::JSON;
    std::string input = "";
    auto result = parseWhitespace()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(result->first.size(), 0);
    ASSERT_EQ(result->second, "");

    input = "abc";
    result = parseWhitespace()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(result->first.size(), 0);
    ASSERT_EQ(result->second, "abc");

    input = " abc";
    result = parseWhitespace()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<char>(result->first.front()), ' ');
    ASSERT_EQ(result->second, "abc");

    input = "\nabc";
    result = parseWhitespace()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<char>(result->first.front()), '\n');
    ASSERT_EQ(result->second, "abc");

    input = "\rabc";
    result = parseWhitespace()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<char>(result->first.front()), '\r');
    ASSERT_EQ(result->second, "abc");

    input = "\tabc";
    result = parseWhitespace()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<char>(result->first.front()), '\t');
    ASSERT_EQ(result->second, "abc");
}
