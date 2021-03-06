//
// Created by coder2k on 22.08.2021.
//

#pragma once

#include "Iterators.hpp"
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <functional>
#include <new>
#include <utility>

namespace c2k {

    class TypeErasedVector final {
    public:
        using Iterator = Iterators::Iterator;
        using ConstIterator = Iterators::ConstIterator;
        template<typename T>
        using TypedIterator = T*;
        template<typename T>
        using ConstTypedIterator = const T*;

    public:
        ~TypeErasedVector();

        [[nodiscard]] Iterator begin() noexcept;
        [[nodiscard]] ConstIterator begin() const noexcept;
        [[nodiscard]] ConstIterator cbegin() const noexcept;

        [[nodiscard]] Iterator end() noexcept;
        [[nodiscard]] ConstIterator end() const noexcept;
        [[nodiscard]] ConstIterator cend() const noexcept;

        template<std::default_initializable T>
        [[nodiscard]] TypedIterator<T> begin() noexcept {
            assert(sizeof(T) == mElementSize);
            assert(alignof(T) == mElementAlignment);
            return static_cast<TypedIterator<T>>(mData);
        }

        template<std::default_initializable T>
        [[nodiscard]] ConstTypedIterator<T> begin() const noexcept {
            assert(sizeof(T) == mElementSize);
            assert(alignof(T) == mElementAlignment);
            return static_cast<ConstTypedIterator<T>>(mData);
        }

        template<std::default_initializable T>
        [[nodiscard]] ConstTypedIterator<T> cbegin() const noexcept {
            return begin<T>();
        }

        template<std::default_initializable T>
        [[nodiscard]] TypedIterator<T> end() noexcept {
            assert(sizeof(T) == mElementSize);
            assert(alignof(T) == mElementAlignment);
            return static_cast<TypedIterator<T>>(mData) + mSize;
        }

        template<std::default_initializable T>
        [[nodiscard]] ConstTypedIterator<T> end() const noexcept {
            assert(sizeof(T) == mElementSize);
            assert(alignof(T) == mElementAlignment);
            return static_cast<ConstTypedIterator<T>>(mData) + mSize;
        }

        template<std::default_initializable T>
        [[nodiscard]] ConstTypedIterator<T> cend() const noexcept {
            return end<T>();
        }

        [[nodiscard]] void* operator[](std::size_t index) noexcept {
            assert(index < mSize);
            return static_cast<std::uint8_t*>(mData) + mElementSizePadded * index;
        }

        [[nodiscard]] const void* operator[](std::size_t index) const noexcept {
            assert(index < mSize);
            return static_cast<std::uint8_t*>(mData) + mElementSizePadded * index;
        }

        void swapElements(std::size_t firstIndex, std::size_t secondIndex) noexcept;

        void resize(std::size_t size) noexcept;

        template<std::default_initializable T>
        [[nodiscard]] T& get(std::size_t index) noexcept {
            assert(sizeof(T) == mElementSize);
            assert(alignof(T) == mElementAlignment);
            assert(index < mSize);
            void* const address = static_cast<std::uint8_t*>(mData) + mElementSizePadded * index;
            return *static_cast<T*>(address);
        }

        template<std::default_initializable T>
        [[nodiscard]] const T& get(std::size_t index) const noexcept {
            assert(sizeof(T) == mElementSize);
            assert(alignof(T) == mElementAlignment);
            assert(index < mSize);
            const void* const address = static_cast<std::uint8_t*>(mData) + mElementSizePadded * index;
            return *static_cast<const T*>(address);
        }

        template<std::default_initializable T>
        void push_back(const T& element) noexcept {
            /* Assert that element has the right data type.
         * Because of the type erasure you can never be 100 % sure though! */
            assert(sizeof(element) == mElementSize);
            assert(alignof(T) == mElementAlignment);
            if (mSize == mCapacity) {
                grow();
            }
            auto address{ static_cast<void*>(static_cast<std::uint8_t*>(mData) + mElementSizePadded * mSize) };
            new (address) T{ element };
            ++mSize;
        }

        template<std::default_initializable T>
        void push_back(T&& element) noexcept {
            /* Assert that element has the right data type.
             * Because of the type erasure you can never be 100 % sure though! */
            assert(sizeof(element) == mElementSize);
            assert(alignof(T) == mElementAlignment);
            if (mSize == mCapacity) {
                grow();
            }
            auto address{ static_cast<void*>(static_cast<std::uint8_t*>(mData) + mElementSizePadded * mSize) };
            new (address) T{ std::forward<T>(element) };
            ++mSize;
        }

        void pop_back() noexcept {
            assert(size() != 0ULL);
            resize(size() - 1);
        }

        template<std::default_initializable T>
        [[nodiscard]] static TypeErasedVector forType() noexcept {
            constexpr auto elementSize{ sizeof(T) };
            constexpr auto elementAlignment{ alignof(T) };
            const auto defaultConstruct{ [](void* address) { new (address) T{}; } };
            const auto destructor{ [](void* address) { static_cast<T*>(address)->~T(); } };
            const auto placementNew{ [](void* address, void* elementToMove) {
                new (address) T{ std::move(*static_cast<T*>(elementToMove)) };
            } };
            const auto swapFunc{ [](void* lhs, void* rhs) {
                using std::swap;
                swap(*static_cast<T*>(lhs), *static_cast<T*>(rhs));
            } };
            return TypeErasedVector{
                elementSize, elementAlignment, defaultConstruct, destructor, placementNew, swapFunc
            };
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
                         std::function<void(void*)> defaultConstruct,
                         std::function<void(void*)> destructor,
                         std::function<void(void*, void*)> placementNew,
                         std::function<void(void*, void*)> swap) noexcept;

        void grow() noexcept;

    private:
        std::size_t mSize{ 0 };
        std::size_t mCapacity{ 0 };
        void* mData{ nullptr };
        const std::size_t mElementSize;
        const std::size_t mElementAlignment;
        const std::size_t mElementSizePadded;// includes padding to fulfill alignment requirements
        const std::function<void(void*)> mDefaultConstruct;
        const std::function<void(void*)> mDestructor;
        const std::function<void(void*, void*)> mPlacementNew;
        const std::function<void(void*, void*)> mSwap;
    };

}// namespace c2k