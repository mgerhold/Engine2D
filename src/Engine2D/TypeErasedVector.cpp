//
// Created by coder2k on 22.08.2021.
//

#include "TypeErasedVector.hpp"
#include <range/v3/all.hpp>

static constexpr std::size_t getPaddedSize(std::size_t size, std::size_t alignment) noexcept {
    auto result{ alignment };
    while (result < size) {
        result += alignment;
    }
    return result;
}

namespace c2k {

    TypeErasedVector::~TypeErasedVector() {
        using ranges::views::ints;
        void* current{ mData };
        for ([[maybe_unused]] auto _ : ints(std::size_t{ 0 }, mSize)) {
            mDestructor(current);
            current = static_cast<std::uint8_t*>(current) + mElementSizePadded;
        }
        ::operator delete[](mData, std::align_val_t{ mElementAlignment });
    }

    TypeErasedVector::Iterator TypeErasedVector::begin() noexcept {
        return Iterator{ mData, mElementSizePadded };
    }

    TypeErasedVector::ConstIterator TypeErasedVector::begin() const noexcept {
        return ConstIterator{ mData, mElementSizePadded };
    }

    TypeErasedVector::ConstIterator TypeErasedVector::cbegin() const noexcept {
        return begin();
    }

    TypeErasedVector::Iterator TypeErasedVector::end() noexcept {
        return Iterator{ static_cast<std::uint8_t*>(mData) + mSize * mElementSizePadded, mElementSizePadded };
    }

    TypeErasedVector::ConstIterator TypeErasedVector::end() const noexcept {
        return ConstIterator{ static_cast<std::uint8_t*>(mData) + mSize * mElementSizePadded, mElementSizePadded };
    }

    TypeErasedVector::ConstIterator TypeErasedVector::cend() const noexcept {
        return end();
    }

    void TypeErasedVector::swapElements(std::size_t firstIndex, std::size_t secondIndex) noexcept {
        void* const firstAddress = static_cast<std::uint8_t*>(mData) + firstIndex * mElementSizePadded;
        void* const secondAddress = static_cast<std::uint8_t*>(mData) + secondIndex * mElementSizePadded;
        mSwap(firstAddress, secondAddress);
    }

    void TypeErasedVector::resize(std::size_t size) noexcept {
        using ranges::views::ints;
        if (size == mSize) {
            return;
        }
        // TODO: maybe only grow to the minimum size that is needed and not only use powers of 2
        while (mCapacity < size) {
            grow();
        }
        if (size > mSize) {
            void* const baseAddress{ static_cast<std::uint8_t*>(mData) + mSize * mElementSizePadded };
            void* currentAddress{ baseAddress };
            for ([[maybe_unused]] auto _ : ints(mSize, size)) {
                mDefaultConstruct(currentAddress);
                currentAddress = static_cast<std::uint8_t*>(currentAddress) + mElementSizePadded;
            }
        } else if (size < mSize) {
            void* const baseAddress{ static_cast<std::uint8_t*>(mData) + size * mElementSizePadded };
            void* currentAddress{ baseAddress };
            for ([[maybe_unused]] auto _ : ints(size, mSize)) {
                mDestructor(currentAddress);
                currentAddress = static_cast<std::uint8_t*>(currentAddress) + mElementSizePadded;
            }
        }
        mSize = size;
    }

    TypeErasedVector::TypeErasedVector(std::size_t elementSize,
                                       std::size_t elementAlignment,
                                       std::function<void(void*)> defaultConstruct,
                                       std::function<void(void*)> destructor,
                                       std::function<void(void*, void*)> placementNew,
                                       std::function<void(void*, void*)> swap) noexcept
        : mElementSize{ elementSize },
          mElementAlignment{ elementAlignment },
          mElementSizePadded{ getPaddedSize(elementSize, elementAlignment) },
          mDefaultConstruct{ std::move(defaultConstruct) },
          mDestructor{ std::move(destructor) },
          mPlacementNew{ std::move(placementNew) },
          mSwap{ std::move(swap) } { }

    void TypeErasedVector::grow() noexcept {
        using ranges::views::ints;
        const std::size_t newCapacity{ mCapacity > 0 ? 2 * mCapacity : 1 };
        void* const newBuffer =
                ::operator new[](newCapacity* mElementSizePadded, std::align_val_t{ mElementAlignment });
        void* writePointer{ newBuffer };
        void* readPointer{ mData };
        for ([[maybe_unused]] auto _ : ints(std::size_t{ 0 }, mSize)) {
            mPlacementNew(writePointer, readPointer);
            readPointer = static_cast<std::uint8_t*>(readPointer) + mElementSizePadded;
            writePointer = static_cast<std::uint8_t*>(writePointer) + mElementSizePadded;
        }
        if (mCapacity > 0) {
            void* current{ mData };
            for ([[maybe_unused]] auto _ : ints(std::size_t{ 0 }, mSize)) {
                mDestructor(current);
                current = static_cast<std::uint8_t*>(current) + mElementSizePadded;
            }
            ::operator delete[](mData, std::align_val_t{ mElementAlignment });
        }
        mData = newBuffer;
        mCapacity = newCapacity;
    }

}// namespace c2k