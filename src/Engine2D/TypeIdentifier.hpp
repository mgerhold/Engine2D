//
// Created by coder2k on 19.07.2021.
//

#pragma once

#include "pch.hpp"

namespace c2k {

    class TypeIdentifier final {
    public:
        using UnderlyingType = std::size_t;

    public:
        template<typename Type>
        [[nodiscard]] static UnderlyingType get() {
            const static UnderlyingType id{ getNext() };
            return id;
        }

    private:
        [[nodiscard]] static UnderlyingType getNext() {
            return mNextId++;
        }

    private:
        static inline UnderlyingType mNextId{ 0 };
    };

}// namespace c2k