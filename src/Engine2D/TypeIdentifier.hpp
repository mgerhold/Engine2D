//
// Created by coder2k on 19.07.2021.
//

#pragma once

#include <spdlog/spdlog.h>
#include <cstddef>

template<auto = []{}>
class TypeIdentifierBase final {
public:
    template<typename T>
    [[nodiscard]] static std::size_t get() {
        const static std::size_t id = getNext();
        return id;
    }

private:
    [[nodiscard]] static std::size_t getNext() {
        return mNextId++;
    }

private:
    static inline std::size_t mNextId{ 0 };
};

using TypeIdentifier = TypeIdentifierBase<>;