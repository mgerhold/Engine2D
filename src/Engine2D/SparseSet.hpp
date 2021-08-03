//
// Created by coder2k on 18.07.2021.
//

#pragma once

#include <spdlog/spdlog.h>
#include <range/v3/all.hpp>
#include <vector>
#include <cassert>
#include <limits>
#include <type_traits>
#include <concepts>

template<typename T, std::unsigned_integral SparseIndex, SparseIndex invalidIndex>
class SparseSet final {
public:
    using value_type = T;

public:
    explicit SparseSet(SparseIndex initialSetSize, SparseIndex initialElementCapacity = SparseIndex{ 0 }) noexcept;

    template<std::convertible_to<T> U>
    void add(SparseIndex index, U&& element) noexcept;
    void remove(SparseIndex index) noexcept;
    void resize(std::size_t size) noexcept;

    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool has(SparseIndex index) const noexcept;
    [[nodiscard]] const T& get(SparseIndex index) const noexcept;
    [[nodiscard]] T& getMutable(SparseIndex index) noexcept;
    [[nodiscard]] auto indices() const noexcept;
    [[nodiscard]] auto elements() const noexcept;
    [[nodiscard]] auto elementsMutable() noexcept;

private:
    std::vector<SparseIndex> mSparseVector;
    std::vector<SparseIndex> mDenseVector;
    std::vector<T> mElementVector;
};

template<typename T, std::unsigned_integral SparseIndex, SparseIndex invalidIndex>
SparseSet<T, SparseIndex, invalidIndex>::SparseSet(SparseIndex initialSetSize, SparseIndex initialElementCapacity) noexcept
    : mSparseVector(initialSetSize, invalidIndex) {
    mDenseVector.reserve(initialElementCapacity);
    mElementVector.reserve(initialElementCapacity);
}

template<typename T, std::unsigned_integral SparseIndex, SparseIndex invalidIndex>
bool SparseSet<T, SparseIndex, invalidIndex>::has(SparseIndex index) const noexcept {
    assert(index < mSparseVector.size() && "Invalid index id.");
    const auto denseIndex = mSparseVector[index];
    return denseIndex < mDenseVector.size();
}

template<typename T, std::unsigned_integral SparseIndex, SparseIndex invalidIndex>
template<std::convertible_to<T> U>
void SparseSet<T, SparseIndex, invalidIndex>::add(SparseIndex index, U&& element) noexcept {
    spdlog::info("mSparseVector.size() == {}", mSparseVector.size());
    assert(index < mSparseVector.size() && "Invalid index id.");
    assert(!has(index));
    mSparseVector[index] = static_cast<SparseIndex>(mDenseVector.size());
    mDenseVector.push_back(index);
    mElementVector.push_back(std::forward<U>(element));
}

template<typename T, std::unsigned_integral SparseIndex, SparseIndex invalidIndex>
const T& SparseSet<T, SparseIndex, invalidIndex>::get(SparseIndex index) const noexcept {
    assert(has(index) && "The given index doesn't have an instance of this element.");
    return mElementVector[mSparseVector[index]];
}

template<typename T, std::unsigned_integral SparseIndex, SparseIndex invalidIndex>
T& SparseSet<T, SparseIndex, invalidIndex>::getMutable(SparseIndex index) noexcept {
    assert(has(index) && "The given index doesn't have an instance of this element.");
    return mElementVector[mSparseVector[index]];
}

template<typename T, std::unsigned_integral SparseIndex, SparseIndex invalidIndex>
void SparseSet<T, SparseIndex, invalidIndex>::remove(SparseIndex index) noexcept {
    using std::swap;
    assert(has(index) && "The given index doesn't have an instance of this element.");
    const auto denseIndex = mSparseVector[index];
    mSparseVector[index] = invalidIndex;
    mSparseVector[mDenseVector.back()] = denseIndex;
    mDenseVector[denseIndex] = mDenseVector.back();
    mDenseVector.resize(mDenseVector.size() - 1);
    mElementVector[denseIndex] = std::move(mElementVector.back());
    mElementVector.resize(mElementVector.size() - 1);
    assert(mDenseVector.size() == mElementVector.size());
}

template<typename T, std::unsigned_integral SparseIndex, SparseIndex invalidIndex>
void SparseSet<T, SparseIndex, invalidIndex>::resize(std::size_t size) noexcept {
    assert(size >= mSparseVector.size());
    mSparseVector.resize(size, invalidIndex);
}

template<typename T, std::unsigned_integral SparseIndex, SparseIndex invalidIndex>
[[nodiscard]] std::size_t SparseSet<T, SparseIndex, invalidIndex>::size() const noexcept {
    return mElementVector.size();
}

template<typename T, std::unsigned_integral SparseIndex, SparseIndex invalidIndex>
[[nodiscard]] auto SparseSet<T, SparseIndex, invalidIndex>::elementsMutable() noexcept {
    return mElementVector | ranges::views::all;
}

template<typename T, std::unsigned_integral SparseIndex, SparseIndex invalidIndex>
auto SparseSet<T, SparseIndex, invalidIndex>::indices() const noexcept {
    return mDenseVector | ranges::views::all;
}
template<typename T, std::unsigned_integral SparseIndex, SparseIndex invalidIndex>
auto SparseSet<T, SparseIndex, invalidIndex>::elements() const noexcept {
    return mElementVector | ranges::views::all;
}