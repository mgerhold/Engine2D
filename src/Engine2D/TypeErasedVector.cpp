//
// Created by coder2k on 22.08.2021.
//

#include "TypeErasedVector.hpp"

static std::size_t getPaddedSize(std::size_t size, std::size_t alignment) noexcept {
    auto result{ alignment };
    while (result < size) {
        result += alignment;
    }
    return result;
}

TypeErasedVector::~TypeErasedVector() {
    for (const auto& element : *this) {
        mDestructor(element);
    }
    ::operator delete[](mData, std::align_val_t{ mElementAlignment });
}

TypeErasedVector::Iterator TypeErasedVector::begin() noexcept {
    return Iterator{ mData, mElementSizePadded };
}

TypeErasedVector::Iterator TypeErasedVector::end() noexcept {
    return Iterator{ static_cast<std::uint8_t*>(mData) + mSize * mElementSizePadded, mElementSizePadded };
}

TypeErasedVector::TypeErasedVector(std::size_t elementSize,
                                   std::size_t elementAlignment,
                                   std::function<void(void*)> destructor,
                                   std::function<void(void*, void*)> placementNew) noexcept
    : mElementSize{ elementSize },
      mElementAlignment{ elementAlignment },
      mElementSizePadded{ getPaddedSize(elementSize, elementAlignment) },
      mDestructor{ std::move(destructor) },
      mPlacementNew{ std::move(placementNew) } { }

void TypeErasedVector::grow() noexcept {
    const std::size_t newCapacity{ mCapacity > 0 ? 2 * mCapacity : 1 };
    void* const newBuffer = ::operator new[](newCapacity* mElementSizePadded, std::align_val_t{ mElementAlignment });
    void* current{ newBuffer };
    for (auto element : *this) {
        mPlacementNew(current, element);
        mDestructor(element);
        current = static_cast<std::uint8_t*>(current) + mElementSizePadded;
    }
    if (mCapacity > 0) {
        ::operator delete[](mData, std::align_val_t{ mElementAlignment });
    }
    mData = newBuffer;
    mCapacity = newCapacity;
}