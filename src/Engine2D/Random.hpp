//
// Created by coder2k on 21.08.2021.
//

#pragma once

#include <glm/glm.hpp>
#include <limits>
#include <random>

namespace c2k {

    class Random {
    public:
        Random() noexcept;

        template<std::integral T>
        [[nodiscard]] T range(T minInclusive, T maxInclusive) noexcept {
            std::uniform_int_distribution<T> distribution{ minInclusive, maxInclusive };
            return distribution(mRandomEngine);
        }

        template<std::floating_point T>
        [[nodiscard]] T range(T minInclusive, T maxExclusive) noexcept {
            std::uniform_real_distribution<T> distribution{ minInclusive, maxExclusive };
            return distribution(mRandomEngine);
        }

        template<std::integral T>
        [[nodiscard]] T range(T maxInclusive) noexcept {
            return range(T{ 0 }, maxInclusive);
        }

        template<std::floating_point T>
        [[nodiscard]] T range(T maxExclusive) noexcept {
            return range(T{ 0 }, maxExclusive);
        }

        template<std::integral T>
        [[nodiscard]] T get() noexcept {
            return range(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
        }

        template<std::floating_point T>
        [[nodiscard]] T get() noexcept {
            return range(T{ 0 }, T{ 1 });
        }

        [[nodiscard]] glm::vec3 unitDirection() noexcept {
            const float radians = range(0.0f, 2.0f * glm::pi<float>());
            return glm::vec3{ glm::cos(radians), glm::sin(radians), 0.0f };
        }

        template<typename T>
        [[nodiscard]] T sign() noexcept {
            return static_cast<T>(range(0, 1) * 2 - 1);
        }

    private:
        std::random_device mRandomDevice;
        std::mt19937_64 mRandomEngine;
    };

}// namespace c2k