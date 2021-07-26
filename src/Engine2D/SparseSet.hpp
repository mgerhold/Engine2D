//
// Created by coder2k on 18.07.2021.
//

#pragma once

#include <range/v3/all.hpp>
#include <vector>
#include <cassert>
#include <limits>
#include <type_traits>
#include <concepts>

template<typename T, std::unsigned_integral SparseIndex>
class SparseSet final {
public:
    using value_type = T;

public:
    SparseSet(SparseIndex initialSetSize, SparseIndex initialElementCapacity = 0) noexcept;

    template<std::convertible_to<T> U>
    void add(SparseIndex index, U&& element) noexcept;
    void remove(SparseIndex index) noexcept;

    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool has(SparseIndex index) const noexcept;
    [[nodiscard]] const T& get(SparseIndex index) const noexcept;
    [[nodiscard]] T& getMutable(SparseIndex index) noexcept;
    [[nodiscard]] auto indices() const noexcept;
    [[nodiscard]] auto elements() const noexcept;
    [[nodiscard]] auto elementsMutable() noexcept;

private:
    static constexpr SparseIndex InvalidEntity = std::numeric_limits<SparseIndex>::max();

private:
    std::vector<SparseIndex> mSparseVector;
    std::vector<SparseIndex> mDenseVector;
    std::vector<T> mElementVector;
};

template<typename T, std::unsigned_integral SparseIndex>
SparseSet<T, SparseIndex>::SparseSet(SparseIndex initialSetSize, SparseIndex initialElementCapacity) noexcept
    : mSparseVector(initialSetSize, InvalidEntity) {
    mDenseVector.reserve(initialElementCapacity);
    mElementVector.reserve(initialElementCapacity);
}

template<typename T, std::unsigned_integral SparseIndex>
bool SparseSet<T, SparseIndex>::has(SparseIndex index) const noexcept {
    assert(index < mSparseVector.size() && "Invalid index id.");
    const auto denseIndex = mSparseVector[index];
    return denseIndex < mDenseVector.size();
}

template<typename T, std::unsigned_integral SparseIndex>
template<std::convertible_to<T> U>
void SparseSet<T, SparseIndex>::add(SparseIndex index, U&& element) noexcept {
    assert(index < mSparseVector.size() && "Invalid index id.");
    assert(!has(index));
    mSparseVector[index] = static_cast<SparseIndex>(mDenseVector.size());
    mDenseVector.push_back(index);
    mElementVector.push_back(std::forward<U>(element));
}

template<typename T, std::unsigned_integral SparseIndex>
const T& SparseSet<T, SparseIndex>::get(SparseIndex index) const noexcept {
    assert(has(index) && "The given index doesn't have an instance of this element.");
    return mElementVector[mSparseVector[index]];
}

template<typename T, std::unsigned_integral SparseIndex>
T& SparseSet<T, SparseIndex>::getMutable(SparseIndex index) noexcept {
    assert(has(index) && "The given index doesn't have an instance of this element.");
    return mElementVector[mSparseVector[index]];
}

template<typename T, std::unsigned_integral SparseIndex>
void SparseSet<T, SparseIndex>::remove(SparseIndex index) noexcept {
    using std::swap;
    assert(has(index) && "The given index doesn't have an instance of this element.");
    const auto denseIndex = mSparseVector[index];
    mSparseVector[index] = InvalidEntity;
    mSparseVector[mDenseVector.back()] = denseIndex;
    mDenseVector[denseIndex] = mDenseVector.back();
    mDenseVector.resize(mDenseVector.size() - 1);
    mElementVector[denseIndex] = std::move(mElementVector.back());
    mElementVector.resize(mElementVector.size() - 1);
    assert(mDenseVector.size() == mElementVector.size());
}

template<typename T, std::unsigned_integral SparseIndex>
[[nodiscard]] std::size_t SparseSet<T, SparseIndex>::size() const noexcept {
    return mElementVector.size();
}

template<typename T, std::unsigned_integral SparseIndex>
[[nodiscard]] auto SparseSet<T, SparseIndex>::elementsMutable() noexcept {
    return mElementVector | ranges::views::all;
}

template<typename T, std::unsigned_integral SparseIndex>
auto SparseSet<T, SparseIndex>::indices() const noexcept {
    return mDenseVector | ranges::views::all;
}
template<typename T, std::unsigned_integral SparseIndex>
auto SparseSet<T, SparseIndex>::elements() const noexcept {
    return mElementVector | ranges::views::all;
}