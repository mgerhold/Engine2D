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

TEST(CombinedParsers, parseWhitespaceAndDrop) {
    using namespace c2k::JSON;
    std::string input = "";
    auto result = parseWhitespaceAndDrop()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(result->first.size(), 0);
    ASSERT_EQ(result->second, "");

    input = "abc";
    result = parseWhitespaceAndDrop()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(result->first.size(), 0);
    ASSERT_EQ(result->second, "abc");

    input = " abc";
    result = parseWhitespaceAndDrop()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(result->first.size(), 0);
    ASSERT_EQ(result->second, "abc");

    input = "\nabc";
    result = parseWhitespaceAndDrop()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(result->first.size(), 0);
    ASSERT_EQ(result->second, "abc");

    input = "\rabc";
    result = parseWhitespaceAndDrop()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(result->first.size(), 0);
    ASSERT_EQ(result->second, "abc");

    input = "\tabc";
    result = parseWhitespaceAndDrop()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(result->first.size(), 0);
    ASSERT_EQ(result->second, "abc");

    input = "   \n   \t\r\r  \tabc";
    result = parseWhitespaceAndDrop()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(result->first.size(), 0);
    ASSERT_EQ(result->second, "abc");
}

TEST(CombinedParsers, parseJSONTrue) {
    using namespace c2k::JSON;
    std::string input = "true";
    auto result = parseJSONTrue()(input);
    ASSERT_TRUE(result);
    ASSERT_TRUE(holds_alternative<JSONTrue>(result->first.front()));

    input = "false";
    result = parseJSONTrue()(input);
    ASSERT_FALSE(result);
}

TEST(CombinedParsers, parseJSONFalse) {
    using namespace c2k::JSON;
    std::string input = "false";
    auto result = parseJSONFalse()(input);
    ASSERT_TRUE(result);
    ASSERT_TRUE(holds_alternative<JSONFalse>(result->first.front()));

    input = "true";
    result = parseJSONFalse()(input);
    ASSERT_FALSE(result);
}

TEST(CombinedParsers, parseJSONNull) {
    using namespace c2k::JSON;
    std::string input = "null";
    auto result = parseJSONNull()(input);
    ASSERT_TRUE(result);
    ASSERT_TRUE(holds_alternative<JSONNull>(result->first.front()));

    input = "not_null";
    result = parseJSONNull()(input);
    ASSERT_FALSE(result);
}

TEST(CombinedParsers, parseJSONValue) {
    using namespace c2k::JSON;
    std::string input = "123abc";
    auto result = parseJSONValue()(input);
    ASSERT_TRUE(result);
    ASSERT_TRUE(get<JSONValue>(result->first.front()).isNumber());
    auto number = get<JSONValue>(result->first.front()).asNumber();// tl::expected<JSONNumber, std::string>
    ASSERT_TRUE(number);
    ASSERT_EQ(number->value, 123.0);
    ASSERT_EQ(result->second, "abc");

    // comparison check
    JSONValue jsonNumberValue{ JSONNumber{ .value{ 123.0 } } };
    ASSERT_EQ(jsonNumberValue, get<JSONValue>(result->first.front()));

    input = "  123   \t\n abc";
    result = parseJSONValue()(input);
    ASSERT_TRUE(result);
    ASSERT_TRUE(get<JSONValue>(result->first.front()).isNumber());
    number = get<JSONValue>(result->first.front()).asNumber();// tl::expected<JSONNumber, std::string>
    ASSERT_TRUE(number);
    ASSERT_EQ(number->value, 123.0);
    ASSERT_EQ(result->second, "abc");

    input = "  \"text\"   \t\n abc";
    result = parseJSONValue()(input);
    ASSERT_TRUE(result);
    ASSERT_TRUE(get<JSONValue>(result->first.front()).isString());
    auto string = get<JSONValue>(result->first.front()).asString();// tl::expected<JSONString, std::string>
    ASSERT_TRUE(string);
    ASSERT_EQ(string->value, "text");
    ASSERT_EQ(result->second, "abc");

    input = "  true   \t\n abc";
    result = parseJSONValue()(input);
    ASSERT_TRUE(result);
    ASSERT_TRUE(get<JSONValue>(result->first.front()).isTrue());
    ASSERT_EQ(result->second, "abc");

    // comparison check
    JSONValue jsonTrueValue{ JSONTrue{} };
    ASSERT_EQ(jsonTrueValue, get<JSONValue>(result->first.front()));

    input = "  false   \t\n abc";
    result = parseJSONValue()(input);
    ASSERT_TRUE(result);
    ASSERT_TRUE(get<JSONValue>(result->first.front()).isFalse());
    ASSERT_EQ(result->second, "abc");

    input = "  null   \t\n abc";
    result = parseJSONValue()(input);
    ASSERT_TRUE(result);
    ASSERT_TRUE(get<JSONValue>(result->first.front()).isNull());
    ASSERT_EQ(result->second, "abc");
}

TEST(CombinedParsers, parseJSONArray) {
    using namespace c2k::JSON;
    std::string input = "[]";
    auto result = parseJSONArray()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONArray>(result->first.front()).values.size(), 0);

    input = "[123]";
    result = parseJSONArray()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONArray>(result->first.front()).values.size(), 1);
    auto expected = JSONArray{ JSONNumber{ 123.0 } };
    ASSERT_EQ(get<JSONArray>(result->first.front()), expected);

    input = "[123, \"text\", true, null]";
    result = parseJSONArray()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONArray>(result->first.front()).values.size(), 4);
    expected = JSONArray{ JSONNumber{ 123.0 }, JSONString{ "text" }, JSONTrue{}, JSONNull{} };
    ASSERT_EQ(get<JSONArray>(result->first.front()), expected);

    input = "[123, \"text\", true, null,]";// trailing comma
    result = parseJSONArray()(input);
    ASSERT_FALSE(result);
}

TEST(CombinedParsers, parseJSONObject) {
    using namespace c2k::JSON;
    std::string input = "{}";
    auto result = parseJSONObject()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONObject>(result->first.front()).pairs.size(), 0);

    input = R"({"key": "value"})";
    result = parseJSONObject()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONObject>(result->first.front()).pairs.size(), 1);
    auto expected = JSONObject{ { JSONString{ "key" }, JSONValue{ JSONString{ "value" } } } };
    ASSERT_EQ(expected, get<JSONObject>(result->first.front()));

    input = R"({"key": "value",
    "name": "bjarne",
    "age": 70,
    "programmer": true})";
    result = parseJSONObject()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONObject>(result->first.front()).pairs.size(), 4);
    expected = JSONObject{
        { JSONString{ "key" }, JSONValue{ JSONString{ "value" } } },
        { JSONString{ "name" }, JSONValue{ JSONString{ "bjarne" } } },
        { JSONString{ "age" }, JSONValue{ JSONNumber{ 70.0 } } },
        { JSONString{ "programmer" }, JSONValue{ JSONTrue{} } },
    };
    ASSERT_EQ(expected, get<JSONObject>(result->first.front()));

    input = R"({"key": "value",
    "name": "bjarne",
    "age": 70,
    123: "invalid key",
    "programmer": true})";
    result = parseJSONObject()(input);
    ASSERT_FALSE(result);
}

TEST(CombinedParsers, nestedParsing) {
    using namespace c2k::JSON;
    std::string input = R"({"key": "value",
    "array": [123, "c++", true],
    "object": {
        "language": "c++",
        "versions": [14, 17, 20]
    }
})";
    auto result = parseJSONValue()(input);
    ASSERT_TRUE(result);
    auto value = get<JSONValue>(result->first.front());
    ASSERT_TRUE(value.isObject());
    auto object = value.asObject().value();
    ASSERT_EQ(object.pairs.size(), 3);
    // clang-format off
    auto expected = JSONObject{
        { JSONString{ "key" }, JSONValue{ JSONString{ "value" } } },
        { JSONString{ "array" }, JSONValue{ JSONArray{ JSONNumber{ 123.0 }, JSONString{ "c++" }, JSONTrue{} } } },
        { JSONString{ "object" }, JSONValue{ JSONObject{
                                                 { JSONString{ "language" }, JSONValue{ JSONString{ "c++" } } },
                                                 { JSONString{ "versions" }, JSONValue{ JSONArray{
                                                                                           JSONNumber{ 14.0 },
                                                                                           JSONNumber{ 17.0 },
                                                                                           JSONNumber{ 20.0 }
                                                                                        }
                                                                             }
                                                 }
                                             }
                                  }
        }
    };
    // clang-format on
    ASSERT_EQ(object, expected);

    input = R"({
"startLifeTime": 3,
"lifeTimeVariation": 0.05,
"particlesPerSecond": 100,
"gravity": {
  "x": 0,
  "y": 0,
  "z": -0.001
},
"startScale": {
  "x": 2,
  "y": 2
},
"endScale": {
  "x": 25,
  "y": 25
},
"startRotationSpeed": 0,
"endRotationSpeed": 0,
"startRotationSpeedVariation": 400,
"endRotationSpeedVariation": 10
})";
    result = parseJSONValue()(input);
    ASSERT_TRUE(result);
}

TEST(CombinedParsers, parseFile) {
    using namespace c2k::JSON;
    const auto json = fromFile(std::filesystem::current_path() / "tests" / "assets.json");
    ASSERT_TRUE(json);
    const auto jsonString = json.value().dump();
    const auto json2 = fromString(jsonString);
    ASSERT_TRUE(json2);
    ASSERT_EQ(json, json2);
}

TEST(CombinedParsers, saveAndReadFiles) {
    using namespace c2k::JSON;
    // clang-format off
    const auto json = JSONValue { JSONObject{
        { JSONString{ "color" }, JSONString{ "blue"} },
        { JSONString{ "age" }, JSONNumber{ 42.0 } },
        { JSONString{ "isCool" }, JSONTrue{ } },
        { JSONString{ "isHot" }, JSONFalse{ } },
        { JSONString{ "nil" }, JSONNull{ } },
        { JSONString{ "list" }, JSONArray{
                JSONValue{ JSONString{ "text" } },
                JSONValue{ JSONNumber{ 99.0 } },
                JSONValue{ JSONTrue{ } },
                JSONValue{ JSONFalse{ } },
                JSONValue{ JSONNull{ } },
                JSONValue{ JSONObject{
                        { JSONString{ "nestedKey" }, JSONValue{ JSONString{ "nestedValue" }}},
                        { JSONString{ "nestedNumber" }, JSONValue{ JSONNumber{ 43.0 }}}
                }},
                JSONValue{ JSONArray{
                        JSONValue{ JSONTrue{} },
                        JSONValue{ JSONNull{} },
                        JSONValue{ JSONNumber{ 44.0 } }
                }}
        } },
        { JSONString{ "nestedObject" }, JSONObject {
                { JSONString{ "hobby" }, JSONValue{ JSONString{ "tennis" }}}
        }}
    }};
    // clang-format on
    const auto filename = std::filesystem::current_path() / "tests" / "saveAndReadFiles_test.json";
    json.dumpToFile(filename);
    const auto readJSON = fromFile(filename);
    ASSERT_TRUE(readJSON);
    ASSERT_EQ(json, readJSON);
}