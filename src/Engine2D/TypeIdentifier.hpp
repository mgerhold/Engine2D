//
// Created by coder2k on 19.07.2021.
//

#pragma once

#include "pch.hpp"

namespace c2k {

    template<typename T>
    class TypeIdentifier final {
    public:
        template<typename Type>
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

}// namespace c2k