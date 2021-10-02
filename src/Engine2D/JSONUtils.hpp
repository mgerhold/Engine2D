//
// Created by coder2k on 28.08.2021.
//

#pragma once

namespace c2k::JSONUtils {
    /*
    template<std::convertible_to<std::string>... Keys>
    [[nodiscard]] bool validate(const nlohmann::json& json, Keys... keys) {
        return (json.contains(keys) && ...);
    }

    template<std::convertible_to<std::string>... Keys>
    [[nodiscard]] bool validateNumbers(const nlohmann::json& json, Keys... keys) {
        return ((json.contains(keys) && json[keys].is_number()) && ...);
    }
*/
}