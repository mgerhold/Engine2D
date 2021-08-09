//
// Created by coder2k on 09.08.2021.
//

#pragma once

#include <functional>
#include <concepts>

template<std::unsigned_integral Entity, typename... Components>
class System final {
public:
    System(auto&& setup, auto&& forEach, auto&& finalize) noexcept
        : mSetup{ std::forward<decltype(setup)>(setup) },
          mForEach{ std::forward<decltype(forEach)>(forEach) },
          mFinalize{ std::forward<decltype(finalize)>(finalize) } { }

    void setup() noexcept {
        mSetup();
    }

    void forEach(auto&& componentView) noexcept {
        for (auto&& tuple : componentView) {
            std::apply([this](auto&&... args) { mForEach(std::forward<decltype(args)>(args)...); },
                       std::forward<decltype(tuple)>(tuple));
        }
    }

    void finalize() noexcept {
        mFinalize();
    }

private:
    std::function<void(void)> mSetup;
    std::function<void(Entity, Components...)> mForEach;
    std::function<void(void)> mFinalize;
};
