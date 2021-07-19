//
// Created by coder2k on 19.07.2021.
//

#pragma once

#include <cstddef>

class TypeIdentifier final {
public:
    template<typename T>
    static std::size_t get() {
        const static std::size_t id = getNext();
        return id;
    }

private:
    static std::size_t getNext() {
        return sNextId++;
    }

private:
    static inline std::size_t sNextId{ 0 };
};