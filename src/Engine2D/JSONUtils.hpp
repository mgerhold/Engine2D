//
// Created by coder2k on 28.08.2021.
//

#pragma once

namespace nlohmann {
    template<>
    struct adl_serializer<std::filesystem::path> {
        static void to_json(json& j, const std::filesystem::path& path) {
            j = path.string();
        }

        static void from_json(const json& j, std::filesystem::path& path) {
            path = j.get<std::string>();
        }
    };
}// namespace nlohmann

namespace c2k::JSONUtils {

    template<std::convertible_to<std::string>... Keys>
    [[nodiscard]] bool validate(const nlohmann::json& json, Keys... keys) {
        return (json.contains(keys) && ...);
    }

    template<std::convertible_to<std::string>... Keys>
    [[nodiscard]] bool validateNumbers(const nlohmann::json& json, Keys... keys) {
        return ((json.contains(keys) && json[keys].is_number()) && ...);
    }

}