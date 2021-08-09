//
// Created by coder2k on 09.08.2021.
//

#pragma once

#include <functional>
#include <concepts>

template<std::unsigned_integral Entity, typename... Components>
class System final {
public:
    template<typename SetupFunction, typename ForEachFunction, typename FinalizeFunction>
    System(SetupFunction&& setup, ForEachFunction&& forEach, FinalizeFunction&& finalize) noexcept
        : mSetup{ std::forward<SetupFunction>(setup) },
          mForEach{ std::forward<ForEachFunction>(forEach) },
          mFinalize{ std::forward<FinalizeFunction>(finalize) } { }

    void setup() noexcept {
        mSetup();
    }

    template<typename View>
    void forEach(View&& components) noexcept {
        for (auto&& tuple : components) {
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
