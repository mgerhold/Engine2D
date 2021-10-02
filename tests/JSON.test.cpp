//
// Created by coder2k on 27.09.2021.
//

#include <JSON/JSON.hpp>
#include <gtest/gtest.h>

TEST(PrimitiveParsers, parseChar) {
    using namespace c2k::JSON::Implementation_;
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
    using namespace c2k::JSON::Implementation_;
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
    using namespace c2k::JSON::Implementation_;
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
    using namespace c2k::JSON::Implementation_;
    std::string input = "01abc";
    auto result = parseDigit()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<char>(result->first.front()), '0');
    ASSERT_EQ(result->second, "1abc");
}

TEST(CombinedParsers, operatorPlus) {
    using namespace c2k::JSON::Implementation_;
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
    using namespace c2k::JSON::Implementation_;
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
    using namespace c2k::JSON::Implementation_;
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
    using namespace c2k::JSON::Implementation_;
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
    using namespace c2k::JSON::Implementation_;
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
    using namespace c2k::JSON::Implementation_;
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
    using namespace c2k::JSON::Implementation_;
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
    using namespace c2k::JSON::Implementation_;
    std::string input = "true";
    auto result = parseJSONTrue()(input);
    ASSERT_TRUE(result);
    ASSERT_TRUE(holds_alternative<JSONTrue>(result->first.front()));

    input = "false";
    result = parseJSONTrue()(input);
    ASSERT_FALSE(result);
}

TEST(CombinedParsers, parseJSONFalse) {
    using namespace c2k::JSON::Implementation_;
    std::string input = "false";
    auto result = parseJSONFalse()(input);
    ASSERT_TRUE(result);
    ASSERT_TRUE(holds_alternative<JSONFalse>(result->first.front()));

    input = "true";
    result = parseJSONFalse()(input);
    ASSERT_FALSE(result);
}

TEST(CombinedParsers, parseJSONNull) {
    using namespace c2k::JSON::Implementation_;
    std::string input = "null";
    auto result = parseJSONNull()(input);
    ASSERT_TRUE(result);
    ASSERT_TRUE(holds_alternative<JSONNull>(result->first.front()));

    input = "not_null";
    result = parseJSONNull()(input);
    ASSERT_FALSE(result);
}

TEST(CombinedParsers, parseJSONValue) {
    using namespace c2k::JSON::Implementation_;
    std::string input = "123abc";
    auto result = parseJSONValue()(input);
    ASSERT_TRUE(result);
    ASSERT_TRUE(get<JSONValue>(result->first.front()).isNumber());
    auto number = get<JSONValue>(result->first.front()).asNumber();// tl::expected<JSONNumber, std::string>
    ASSERT_TRUE(number);
    ASSERT_EQ(number.value(), 123.0);
    ASSERT_EQ(result->second, "abc");

    // comparison check
    JSONValue jsonNumberValue{ 123.0 };
    ASSERT_EQ(jsonNumberValue, get<JSONValue>(result->first.front()));

    input = "  123   \t\n abc";
    result = parseJSONValue()(input);
    ASSERT_TRUE(result);
    ASSERT_TRUE(get<JSONValue>(result->first.front()).isNumber());
    number = get<JSONValue>(result->first.front()).asNumber();// tl::expected<JSONNumber, std::string>
    ASSERT_TRUE(number);
    ASSERT_EQ(number.value(), 123.0);
    ASSERT_EQ(result->second, "abc");

    input = "  \"text\"   \t\n abc";
    result = parseJSONValue()(input);
    ASSERT_TRUE(result);
    ASSERT_TRUE(get<JSONValue>(result->first.front()).isString());
    auto string = get<JSONValue>(result->first.front()).asString();// tl::expected<JSONString, std::string>
    ASSERT_TRUE(string);
    ASSERT_EQ(string.value(), "text");
    ASSERT_EQ(result->second, "abc");

    input = "  true   \t\n abc";
    result = parseJSONValue()(input);
    ASSERT_TRUE(result);
    ASSERT_TRUE(get<JSONValue>(result->first.front()).isTrue());
    ASSERT_EQ(result->second, "abc");

    // comparison check
    JSONValue jsonTrueValue{ true };
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
    using namespace c2k::JSON::Implementation_;
    std::string input = "[]";
    auto result = parseJSONArray()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONArray>(result->first.front()).values.size(), 0);
    JSONValue expected = JSONArray{};
    ASSERT_EQ(get<JSONArray>(result->first.front()), expected);

    input = "[123]";
    result = parseJSONArray()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONArray>(result->first.front()).values.size(), 1);
    expected = JSONArray{ 123.0 };
    ASSERT_EQ(get<JSONArray>(result->first.front()), expected);

    input = "[123, \"text\", true, null]";
    result = parseJSONArray()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONArray>(result->first.front()).values.size(), 4);
    expected = JSONArray{ 123.0, "text", true, nullptr };
    spdlog::info(expected.dump());
    ASSERT_EQ(get<JSONArray>(result->first.front()), expected);

    input = "[123, \"text\", true, null,]";// trailing comma
    result = parseJSONArray()(input);
    ASSERT_FALSE(result);
}

TEST(CombinedParsers, parseJSONObject) {
    using namespace c2k::JSON::Implementation_;
    std::string input = "{}";
    auto result = parseJSONObject()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONObject>(result->first.front()).pairs.size(), 0);

    input = R"({"key": "value"})";
    result = parseJSONObject()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONObject>(result->first.front()).pairs.size(), 1);
    JSONValue expected = { { "key", "value" } };
    ASSERT_EQ(expected, get<JSONObject>(result->first.front()));

    input = R"({"key": "value",
    "name": "bjarne",
    "age": 70,
    "programmer": true})";
    result = parseJSONObject()(input);
    ASSERT_TRUE(result);
    ASSERT_EQ(get<JSONObject>(result->first.front()).pairs.size(), 4);
    expected = JSONObject{
        { "key", "value" },
        { "name", "bjarne" },
        { "age", 70.0 },
        { "programmer", true },
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
    using namespace c2k::JSON::Implementation_;
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
    JSONValue expected = {
        { "key", "value" },
        { "array", JSONArray{ 123, "c++", true } },
        { "object", {
                { "language", "c++" },
                { "versions", JSONArray{ 14, 17, 20 } }
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
    using namespace c2k;
    using namespace c2k::JSON::Implementation_;
    const auto json = JSON::fromFile(std::filesystem::current_path() / "tests" / "assets.json");
    ASSERT_TRUE(json);
    const auto jsonString = json.value().dump();
    const auto json2 = JSON::fromString(jsonString);
    ASSERT_TRUE(json2);
    ASSERT_EQ(json, json2);
}

TEST(CombinedParsers, saveAndReadFiles) {
    using namespace c2k;
    // clang-format off
    const JSON::Value json = {
        { "color", "blue" },
        { "age", 42.0 },
        { "isCool", true },
        { "isHot", false },
        { "nil", nullptr },
        { "list", JSON::Array{
                "text",
                99.0,
                true,
                false,
                nullptr,
                {
                    { "nestedKey", "nestedValue" },
                    { "nestedNumber", 43.0 }
                },
                JSON::Array{
                        true,
                        nullptr,
                        44.0
                }
        } },
        { "nestedObject", {
                { "hobby", "tennis" }
            }
        }
    };
    // clang-format on
    const auto filename = std::filesystem::current_path() / "tests" / "saveAndReadFiles_test.json";
    json.dumpToFile(filename);
    const auto readJSON = JSON::fromFile(filename);
    ASSERT_TRUE(readJSON);
    ASSERT_EQ(json, readJSON);
}

TEST(CombinedParsers, jsonLiterals) {
    using namespace c2k::JSON;
    const auto json = R"(
  {
    "happy": true,
    "pi": 3.141
  }
  )"_asjson;
    ASSERT_TRUE(json);
    const Value expected = { { "happy", true }, { "pi", 3.141 } };
    ASSERT_EQ(json.value(), expected);
}

TEST(CombinedParsers, convertingPrimitives) {
    using namespace c2k;
    const JSON::Value jsonNumber{ 42 };
    ASSERT_TRUE(jsonNumber.isNumber());
    ASSERT_EQ(jsonNumber.asNumber().value(), 42);

    const JSON::Value jsonString{ "json test" };
    ASSERT_TRUE(jsonString.isString());
    ASSERT_EQ(jsonString.asString().value(), "json test");
}

struct Person {
    std::string name;
    std::string address;
    int age;

    [[nodiscard]] bool operator==(const Person&) const = default;
};

C2K_JSON_DEFINE_TYPE(Person, name, address, age);

struct Course {
    std::string name;
    Person teacher;

    [[nodiscard]] bool operator==(const Course&) const = default;
};

C2K_JSON_DEFINE_TYPE(Course, name, teacher);

TEST(CombinedParsers, customTypesToJSON) {
    using namespace c2k;
    Person person{ .name{ "Edsger W. Dijkstra" }, .address{ "Test Street" }, .age{ 42 } };
    Course course{ .name{ "Computer Science" }, .teacher{ person } };
    const JSON::Value json{ course };
    spdlog::info(json.dump());
    ASSERT_TRUE(json.isObject());
    ASSERT_TRUE(json.containsKey("name"));
    ASSERT_EQ(json.at("name")->asString().value(), "Computer Science");
    ASSERT_TRUE(json.at("teacher")->containsKey("name"));
    ASSERT_EQ(json.at("teacher")->at("name")->asString().value(), "Edsger W. Dijkstra");
    ASSERT_TRUE(json.at("teacher")->containsKey("address"));
    ASSERT_EQ(json.at("teacher")->at("address")->asString().value(), "Test Street");
    ASSERT_TRUE(json.at("teacher")->containsKey("age"));
    ASSERT_EQ(json.at("teacher")->at("age")->asNumber().value(), 42);
}

TEST(CombinedParsers, fromJSON) {
    using namespace c2k;
    JSON::Value json{ 42 };
    double number;
    auto result = JSON::fromJSON(json, number);
    ASSERT_TRUE(result);
    ASSERT_EQ(number, 42);

    json = JSON::Value{ "text" };
    std::string string;
    result = JSON::fromJSON(json, string);
    ASSERT_TRUE(result);
    ASSERT_EQ(string, "text");

    json = JSON::Value{ true };
    bool boolVal;
    result = JSON::fromJSON(json, boolVal);
    ASSERT_TRUE(result);
    ASSERT_EQ(boolVal, true);

    Person person{ .name{ "Edsger W. Dijkstra" }, .address{ "Test Street" }, .age{ 42 } };
    json = JSON::Value{ person };
    Person deserializedPerson;
    result = fromJSON(json, deserializedPerson);
    ASSERT_TRUE(result);
    ASSERT_EQ(deserializedPerson, person);
}

struct Date {
    int year;
    int month;
    int day;

    bool operator==(const Date&) const = default;
};

C2K_JSON_DEFINE_TYPE(Date, year, month, day);

struct Address {
    std::string street;
    std::string city;
    int zipCode;

    bool operator==(const Address&) const = default;
};

C2K_JSON_DEFINE_TYPE(Address, street, city, zipCode);

struct Student {
    std::string name;
    Address address;
    Date dateOfBirth;
    int matriculationNumber;

    bool operator==(const Student&) const = default;
};

C2K_JSON_DEFINE_TYPE(Student, name, address, dateOfBirth, matriculationNumber);

TEST(CombinedParsers, minimalCompleteExample) {
    using namespace c2k;
    // create example dataset
    Student student{ .name{ "Debbie B. Watson" },
                     .address{ .street{ "2305 Elm Drive" }, .city{ "Garden City, NY" }, .zipCode{ 11530 } },
                     .dateOfBirth{ .year{ 1988 }, .month{ 10 }, .day{ 19 } },
                     .matriculationNumber{ 73390599 } };

    // convert to JSON value
    const JSON::Value json{ student };

    // serialize to string
    const auto serialized = json.dump();
    spdlog::info(serialized);

    // deserialize back from string
    const tl::expected<JSON::Value, std::string> deserialized = JSON::fromString(serialized);

    ASSERT_TRUE(deserialized);    // did parsing and deserializing work?
    ASSERT_EQ(json, deserialized);// compare original and deserialized data
}

TEST(CombinedParsers, stdVector) {
    using namespace c2k;
    const auto result = JSON::fromString(R"(["this", "is", "a", "test"])");
    ASSERT_TRUE(result);
    const auto json = result.value();
    spdlog::info(json.dump());

    std::vector<std::string> vector;
    const auto deserializationResult = JSON::fromJSON(json, vector);
    ASSERT_TRUE(deserializationResult);
    ASSERT_EQ(vector.size(), 4);
    ASSERT_EQ(vector[0], "this");
    ASSERT_EQ(vector[1], "is");
    ASSERT_EQ(vector[2], "a");
    ASSERT_EQ(vector[3], "test");

    const std::vector<std::string> words{ "lorem", "ipsum", "dolor", "sit", "amet" };
    const auto wordsJSON = JSON::Value{ words };
    ASSERT_TRUE(wordsJSON.isArray());
    std::vector<std::string> reconvertedWords;
    const auto reconversionResult = JSON::fromJSON(words, reconvertedWords);
    ASSERT_TRUE(reconversionResult);
    ASSERT_EQ(words, reconvertedWords);
}

struct Language {
    std::string name;
    std::vector<std::string> versions;
    std::vector<int> versionNumbers;

    bool operator==(const Language&) const = default;
};

C2K_JSON_DEFINE_TYPE(Language, name, versions, versionNumbers);

TEST(CombinedParsers, stdVectorInsideStruct) {
    using namespace c2k;
    const Language cpp{ .name{ "C++" },
                        .versions{ { "C++ 11", "C++ 14", "C++ 17", "C++ 20", "C++ 23" } },
                        .versionNumbers{ { 11, 14, 17, 20, 23 } } };
    const auto json = JSON::Value{ cpp };
    spdlog::info(json.dump());

    Language reconvertedLanguage;
    const auto result = fromJSON(json, reconvertedLanguage);
    ASSERT_TRUE(result);
    ASSERT_EQ(reconvertedLanguage, cpp);
}

struct University {
    std::string name;
    std::vector<Student> students;
    bool isPublic;

    bool operator==(const University&) const = default;
};

C2K_JSON_DEFINE_TYPE(University, name, students, isPublic);

TEST(CombinedParsers, vectorOfUserDefinedStruct) {
    using namespace c2k;
    const University university{
        .name{ "University of Applied Sciences New York" },
        .students{ { Student{ .name{ "Willie R. Parsons" },
                              .address{ .street{ "755 Maple Lane" }, .city{ "Birmingham" }, .zipCode{ 35203 } },
                              .dateOfBirth{ .year{ 1990 }, .month{ 1 }, .day{ 29 } },
                              .matriculationNumber{ 78574340 } },
                     Student{ .name{ "Amy R. Johnson" },
                              .address{ .street{ "403 Timbercrest Road" }, .city{ "Juneau" }, .zipCode{ 99801 } },
                              .dateOfBirth{ .year{ 1996 }, .month{ 1 }, .day{ 4 } },
                              .matriculationNumber{ 88542479 } } } },
        .isPublic{ true }
    };
    const auto json = JSON::Value{ university };
    const auto filename = std::filesystem::current_path() / "tests" / "university.json";
    const auto saveResult = json.dumpToFile(filename);
    ASSERT_TRUE(saveResult);
    const auto deserializationResult = JSON::fromFile(filename).and_then(JSON::as<University>);
    ASSERT_TRUE(deserializationResult);
    ASSERT_EQ(university, deserializationResult.value());

    ASSERT_FALSE(JSON::fromFile(filename).and_then(JSON::as<Student>));
}