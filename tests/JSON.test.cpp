//
// Created by coder2k on 27.09.2021.
//

#include <JSON/JSON.hpp>
#include <ScopedTimer.hpp>
#include <FileUtils/FileUtils.hpp>
#include <gtest/gtest.h>

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

// remove comments of the following test as soon as nlohmann-json is not longer included
/*TEST(CombinedParsers, jsonLiterals) {
    using namespace c2k::JSON;
    const auto json = R"(
  {
    "happy": true,
    "pi": 3.141
  }
  )"_json;
    ASSERT_TRUE(json);
    const Value expected = { { "happy", true }, { "pi", 3.141 } };
    ASSERT_EQ(json.value(), expected);
}*/

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
    auto result = fromJSON(json, number);
    ASSERT_TRUE(result);
    ASSERT_EQ(number, 42);

    json = JSON::Value{ "text" };
    std::string string;
    result = fromJSON(json, string);
    ASSERT_TRUE(result);
    ASSERT_EQ(string, "text");

    json = JSON::Value{ true };
    bool boolVal;
    result = fromJSON(json, boolVal);
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
    const auto deserializationResult = fromJSON(json, vector);
    ASSERT_TRUE(deserializationResult);
    ASSERT_EQ(vector.size(), 4);
    ASSERT_EQ(vector[0], "this");
    ASSERT_EQ(vector[1], "is");
    ASSERT_EQ(vector[2], "a");
    ASSERT_EQ(vector[3], "test");

    const std::vector<std::string> words{ "lorem", "ipsum", "dolor", "sit", "amet" };
    const auto wordsJSON = JSON::Value{ words };
    ASSERT_TRUE(wordsJSON.isArray());
    const auto reconversionResult = wordsJSON.as<std::vector<std::string>>();
    ASSERT_TRUE(reconversionResult);
    ASSERT_EQ(words, reconversionResult.value());
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

struct SizeJSON {
    int w;
    int h;
};
C2K_JSON_DEFINE_TYPE(SizeJSON, w, h);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SizeJSON, w, h);

struct RectJSON {
    int x;
    int y;
    int w;
    int h;
};
C2K_JSON_DEFINE_TYPE(RectJSON, x, y, w, h);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RectJSON, x, y, w, h);

struct FrameJSON {
    RectJSON frame;
    SizeJSON sourceSize;
};
C2K_JSON_DEFINE_TYPE(FrameJSON, frame, sourceSize);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FrameJSON, frame, sourceSize);

struct MetaJSON {
    SizeJSON size;
};
C2K_JSON_DEFINE_TYPE(MetaJSON, size);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MetaJSON, size);

struct SpriteSheetJSON {
    std::vector<FrameJSON> frames;
    MetaJSON meta;
};
C2K_JSON_DEFINE_TYPE(SpriteSheetJSON, frames, meta);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SpriteSheetJSON, frames, meta);

TEST(CombinedParsers, largerJSONfile) {
    using namespace c2k;
    const auto filename = std::filesystem::current_path() / "tests" / "spritesheet_test.json";
    tl::expected<std::string, std::string> fileReadResult;
    tl::expected<JSON::Value, std::string> parseResult;
    nlohmann::json json;
    std::uint32_t numIterations{ 1U };
    SpriteSheetJSON spriteSheet;
    {
        auto timer = ScopedTimer{ "Reading text file from disk" };
        fileReadResult = FileUtils::readTextFile(filename);
    }
    {
        for (decltype(numIterations) i{ 0U }; i < numIterations; ++i) {
            {
                auto timer = ScopedTimer{ "[c2k] Parsing JSON file" };
                parseResult = JSON::fromString(fileReadResult.value());
            }
            {
                auto timer = ScopedTimer{ "[nlohmann] Parsing JSON file" };
                json = nlohmann::json::parse(fileReadResult.value());
            }
        }
    }
    {
        for (decltype(numIterations) i{ 0U }; i < numIterations; ++i) {
            {
                auto timer = ScopedTimer{ "[c2k] Deserializing JSON value" };
                spriteSheet = parseResult->as<SpriteSheetJSON>().value();
            }
            {
                auto timer = ScopedTimer{ "[nlohmann] Deserializing JSON value" };
                json.get_to(spriteSheet);
            }
        }
    }
    spdlog::info("Read {} sprite frames", spriteSheet.frames.size());
    ASSERT_EQ(spriteSheet.frames.size(), 60);
    ScopedTimer::logResults();
}