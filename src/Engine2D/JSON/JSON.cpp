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
            if (holds_alternative<std::monostate>(mData) || holds_alternative<std::monostate>(other.mData)) {
                return false;
            }
            return mData == other.mData;
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
            const auto& object = get<JSONObject>(mData);
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
            const auto& object = get<JSONObject>(mData);
            const auto findIterator = std::find_if(object.pairs.cbegin(), object.pairs.cend(),
                                                   [&key](const auto& pair) { return pair.first.value == key; });
            return findIterator != object.pairs.cend();
        }

    }// namespace Implementation_

    tl::expected<Value, std::string> fromString(const std::string& input) noexcept {
        const auto result = Implementation_::parseJSONValue()(input);
        if (!result) {
            // TODO: improve error message
            return tl::unexpected{ fmt::format("Unable to parse string") };
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
