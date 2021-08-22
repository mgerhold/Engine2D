//
// Created by coder2k on 22.08.2021.
//

#pragma once

#include "pch.hpp"

class TypeErasedVector final {
private:
#include "TypeErasedVectorIterator.inc"

public:
    using Iterator = TypeErasedVectorIterator;
    template<typename T>
    using TypedIterator = TypedTypeErasedVectorIterator<T>;

public:
    ~TypeErasedVector();

    [[nodiscard]] Iterator begin() noexcept;
    [[nodiscard]] Iterator end() noexcept;
    template<typename T>
    [[nodiscard]] TypedIterator<T> begin() noexcept {
        assert(sizeof(T) == mElementSize);
        assert(alignof(T) == mElementAlignment);
        return TypedIterator<T>{ mData, mElementSizePadded };
    }
    template<typename T>
    [[nodiscard]] TypedIterator<T> end() noexcept {
        assert(sizeof(T) == mElementSize);
        assert(alignof(T) == mElementAlignment);
        return TypedIterator<T>{ static_cast<std::uint8_t*>(mData) + mSize * mElementSizePadded, mElementSizePadded };
    }

    [[nodiscard]] void* operator[](std::size_t index) noexcept {
        assert(index < mSize);
        return static_cast<std::uint8_t*>(mData) + mElementSizePadded * index;
    }

    [[nodiscard]] const void* operator[](std::size_t index) const noexcept {
        assert(index < mSize);
        return static_cast<std::uint8_t*>(mData) + mElementSizePadded * index;
    }

    template<typename T>
    [[nodiscard]] T& get(std::size_t index) noexcept {
        assert(sizeof(T) == mElementSize);
        assert(alignof(T) == mElementAlignment);
        assert(index < mSize);
        void* const address = static_cast<std::uint8_t*>(mData) + mElementSizePadded * index;
        return *static_cast<T*>(address);
    }

    template<typename T>
    [[nodiscard]] const T& get(std::size_t index) const noexcept {
        assert(sizeof(T) == mElementSize);
        assert(alignof(T) == mElementAlignment);
        assert(index < mSize);
        const void* const address = static_cast<std::uint8_t*>(mData) + mElementSizePadded * index;
        return *static_cast<const T*>(address);
    }

    template<typename T>
    void push_back(T&& element) noexcept {
        /* Assert that element has the right data type.
         * Because of the type erasure you can never be 100 % sure though! */
        assert(sizeof(element) == mElementSize);
        assert(alignof(T) == mElementAlignment);
        if (mSize == mCapacity) {
            grow();
        }
        const auto address{ static_cast<T*>(
                static_cast<void*>(static_cast<std::uint8_t*>(mData) + mElementSizePadded * mSize)) };
        new (address) T{ std::forward<T>(element) };
        ++mSize;
    }

    template<typename T>
    [[nodiscard]] static TypeErasedVector forType() noexcept {
        constexpr auto elementSize{ sizeof(T) };
        constexpr auto elementAlignment{ alignof(T) };
        const auto destructor{ [](void* address) { static_cast<T*>(address)->~T(); } };
        const auto placementNew{ [](void* address, void* elementToMove) {
            new (address) T{ std::move(*static_cast<T*>(elementToMove)) };
        } };
        return TypeErasedVector{ elementSize, elementAlignment, destructor, placementNew };
    }

    [[nodiscard]] std::size_t capacity() const noexcept {
        return mCapacity;
    }
    [[nodiscard]] std::size_t size() const noexcept {
        return mSize;
    }

    void reserve(std::size_t capacity) noexcept {
        // TODO: maybe don't only use powers of 2
        while (mCapacity < capacity) {
            grow();
        }
    }

private:
    TypeErasedVector(std::size_t elementSize,
                     std::size_t elementAlignment,
                     std::function<void(void*)> destructor,
                     std::function<void(void*, void*)> placementNew) noexcept;

    void grow() noexcept;

private:
    std::size_t mSize{ 0 };
    std::size_t mCapacity{ 0 };
    void* mData{ nullptr };
    const std::size_t mElementSize;
    const std::size_t mElementAlignment;
    const std::size_t mElementSizePadded;// includes padding to fulfill alignment requirements
    const std::function<void(void*)> mDestructor;
    const std::function<void(void*, void*)> mPlacementNew;
};