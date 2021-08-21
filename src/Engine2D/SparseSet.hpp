//
// Created by coder2k on 18.07.2021.
//

#pragma once

#include "pch.hpp"

namespace c2k {

    template<typename T, std::unsigned_integral SparseIndex, SparseIndex invalidIndex>
    class SparseSet final {
    public:
        using value_type = T;

    public:
        explicit SparseSet(SparseIndex initialSetSize, SparseIndex initialElementCapacity = SparseIndex{ 0 }) noexcept
            : mSparseVector(initialSetSize, invalidIndex) {
            mDenseVector.reserve(initialElementCapacity);
            mElementVector.reserve(initialElementCapacity);
        }

        template<std::convertible_to<T> Component>
        void add(SparseIndex index, Component&& element) noexcept {
            assert(index < mSparseVector.size() && "Invalid index id.");
            assert(!has(index));
            mSparseVector[index] = static_cast<SparseIndex>(mDenseVector.size());
            mDenseVector.push_back(index);
            mElementVector.push_back(std::forward<Component>(element));
        }
        void remove(SparseIndex index) noexcept {
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
        void resize(std::size_t size) noexcept {
            assert(size >= mSparseVector.size());
            mSparseVector.resize(size, invalidIndex);
        }

        [[nodiscard]] std::size_t size() const noexcept {
            return mElementVector.size();
        }
        [[nodiscard]] bool has(SparseIndex index) const noexcept {
            assert(index < mSparseVector.size() && "Invalid index id.");
            const auto denseIndex = mSparseVector[index];
            return denseIndex < mDenseVector.size();
        }
        [[nodiscard]] const T& get(SparseIndex index) const noexcept {
            assert(has(index) && "The given index doesn't have an instance of this element.");
            return mElementVector[mSparseVector[index]];
        }
        [[nodiscard]] T& getMutable(SparseIndex index) noexcept {
            assert(has(index) && "The given index doesn't have an instance of this element.");
            return mElementVector[mSparseVector[index]];
        }
        [[nodiscard]] auto indices() const noexcept {
            return mDenseVector | ranges::views::all;
        }
        [[nodiscard]] auto elements() const noexcept {
            return mElementVector | ranges::views::all;
        }
        [[nodiscard]] auto elementsMutable() noexcept {
            return mElementVector | ranges::views::all;
        }

    private:
        std::vector<SparseIndex> mSparseVector;
        std::vector<SparseIndex> mDenseVector;
        std::vector<T> mElementVector;
    };

}// namespace c2k