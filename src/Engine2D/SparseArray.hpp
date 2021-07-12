//
// Created by coder2k on 12.07.2021.
//

#pragma once

#include <vector>
#include <limits>
#include <concepts>
#include <cassert>

template<std::default_initializable T>
class SparseArray final {
public:
    using Iterator = typename std::vector<T>::iterator;
    using ConstIterator = typename std::vector<T>::const_iterator;

public:
    explicit SparseArray(std::size_t initialIndexCapacity = 0, std::size_t initialElementCapacity = 0) noexcept;
    SparseArray(const SparseArray&) = delete;
    SparseArray& operator=(const SparseArray&) = delete;
    // TODO: Move semantics

    [[nodiscard]] const T& operator[](std::size_t index) const;
    [[nodiscard]] T& operator[](std::size_t index);

    [[nodiscard]] Iterator begin() {
        return mElements.begin();
    }

    [[nodiscard]] Iterator end() {
        return mElements.end();
    }

    [[nodiscard]] ConstIterator begin() const {
        return mElements.begin();
    }

    [[nodiscard]] ConstIterator end() const {
        return mElements.end();
    }

    [[nodiscard]] ConstIterator cbegin() const {
        return mElements.cbegin();
    }

    [[nodiscard]] ConstIterator cend() const {
        return mElements.cend();
    }

    void resizeIndexContainer(const std::size_t newIndexCapacity) {
        assert(newIndexCapacity >= mIndices.size());
        mIndices.resize(newIndexCapacity, InvalidIndex);
    }

    void reserveElementContainer(const std::size_t newElementCapacity) {
        assert(newElementCapacity >= mElements.capacity());
        mElements.reserve(newElementCapacity);
    }

    [[nodiscard]] std::size_t indexContainerSize() const { return mIndices.size(); }
    [[nodiscard]] std::size_t elementContainerCapacity() const { return mElements.capacity(); }
    [[nodiscard]] std::size_t elementContainerSize() const { return mElements.size(); }

public:
    static constexpr std::size_t InvalidIndex = std::numeric_limits<std::size_t>::max();

private:
    std::vector<std::size_t> mIndices;
    std::vector<T> mElements;
};

template<std::default_initializable T>
const T& SparseArray<T>::operator[](const std::size_t index) const {
    assert(index < mIndices.size());
    assert(mIndices[index] != InvalidIndex);
    assert(mIndices[index] < mElements.size());
    return mElements[mIndices[index]];
}

template<std::default_initializable T>
T& SparseArray<T>::operator[](const std::size_t index) {
    assert(index < mIndices.size());
    if (mIndices[index] == InvalidIndex) {
        mElements.emplace_back();
        mIndices[index] = mElements.size() - 1;
        return mElements.back();
    }
    return mElements[mIndices[index]];
}

template<std::default_initializable T>
SparseArray<T>::SparseArray(const std::size_t initialIndexCapacity, const std::size_t initialElementCapacity) noexcept
    : mIndices(initialIndexCapacity, InvalidIndex),
      mElements(initialElementCapacity) { }