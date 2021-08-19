//
// Created by coder2k on 09.08.2021.
//

#pragma once

#include <functional>
#include <concepts>

template<std::unsigned_integral Entity,
         typename SetupFunction,
         typename ForEachFunction,
         typename FinalizeFunction,
         typename... Components>
class System final {
public:
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

    void forEach() noexcept {
        mForEach();
    }

    void finalize() noexcept {
        mFinalize();
    }

private:
    SetupFunction mSetup;
    ForEachFunction mForEach;
    FinalizeFunction mFinalize;
};
