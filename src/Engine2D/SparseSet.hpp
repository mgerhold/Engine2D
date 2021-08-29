//
// Created by coder2k on 18.07.2021.
//

#pragma once

#include "TypeErasedVector.hpp"
#include "Tag.hpp"

namespace c2k {

    template<std::unsigned_integral SparseIndex, SparseIndex invalidIndex>
    class SparseSet final {
    public:
        template<typename T>
        explicit SparseSet(Tag<T>,
                           SparseIndex initialSetSize,
                           SparseIndex initialElementCapacity = SparseIndex{ 0 }) noexcept
            : mSparseVector(initialSetSize, invalidIndex),// <- no unified initialization because of ctor ambiguity
              mElementVector{ TypeErasedVector::forType<T>() } {
            mDenseVector.reserve(initialElementCapacity);
            mElementVector.reserve(initialElementCapacity);
        }

        [[nodiscard]] auto begin() noexcept {
            return mElementVector.begin();
        }

        [[nodiscard]] auto begin() const noexcept {
            return mElementVector.cbegin();
        }

        [[nodiscard]] auto cbegin() const noexcept {
            return mElementVector.cbegin();
        }

        [[nodiscard]] auto end() noexcept {
            return mElementVector.end();
        }

        [[nodiscard]] auto end() const noexcept {
            return mElementVector.cend();
        }

        [[nodiscard]] auto cend() const noexcept {
            return mElementVector.cend();
        }

        [[nodiscard]] auto indicesBegin() noexcept {
            return mDenseVector.begin();
        }

        [[nodiscard]] auto indicesBegin() const noexcept {
            return mDenseVector.cbegin();
        }

        [[nodiscard]] auto indicesEnd() noexcept {
            return mDenseVector.end();
        }

        [[nodiscard]] auto indicesEnd() const noexcept {
            return mDenseVector.cend();
        }

        template<typename Component>
        void add(SparseIndex index, Component&& element) noexcept {
            assert(index < mSparseVector.size() && "Invalid index id.");
            assert(!has(index));
            mSparseVector[index] = static_cast<SparseIndex>(mDenseVector.size());
            mDenseVector.push_back(index);
            mElementVector.push_back(std::forward<decltype(element)>(element));
        }

        void remove(SparseIndex index) noexcept {
            using std::swap;
            assert(has(index) && "The given index doesn't have an instance of this element.");
            const auto denseIndex = mSparseVector[index];
            mSparseVector[index] = invalidIndex;
            mSparseVector[mDenseVector.back()] = denseIndex;
            mDenseVector[denseIndex] = mDenseVector.back();
            mDenseVector.resize(mDenseVector.size() - 1);
            mElementVector.swapElements(denseIndex, mElementVector.size() - 1);
            mElementVector.resize(mElementVector.size() - 1);
            assert(mDenseVector.size() == mElementVector.size());
        }

        void resize(std::size_t size) noexcept {
            assert(size >= mSparseVector.size());
            mSparseVector.resize(size, invalidIndex);
        }

        [[nodiscard]] std::size_t size() const noexcept {
            return mSparseVector.size();
        }
        [[nodiscard]] std::size_t elementCount() const noexcept {
            return mElementVector.size();
        }
        [[nodiscard]] bool has(SparseIndex index) const noexcept {
            assert(index < mSparseVector.size() && "Invalid index id.");
            const auto denseIndex = mSparseVector[index];
            return denseIndex < mDenseVector.size();
        }

        template<typename T>
        [[nodiscard]] const T& get(SparseIndex index) const noexcept {
            assert(has(index) && "The given index doesn't have an instance of this element.");
            return mElementVector.get<T>(mSparseVector[index]);
        }

        [[nodiscard]] void* getTypeErasedMutable(SparseIndex index) noexcept {
            assert(has(index) && "The given index doesn't have an instance of this element.");
            return mElementVector[mSparseVector[index]];
        }

        [[nodiscard]] const void* getTypeErased(SparseIndex index) const noexcept {
            assert(has(index) && "The given index doesn't have an instance of this element.");
            return mElementVector[mSparseVector[index]];
        }

        template<typename T>
        [[nodiscard]] T& getMutable(SparseIndex index) noexcept {
            assert(has(index) && "The given index doesn't have an instance of this element.");
            return mElementVector.get<T>(mSparseVector[index]);
        }
        [[nodiscard]] auto indices() const noexcept {
            return mDenseVector | ranges::views::all;
        }

        template<typename T>
        [[nodiscard]] auto elements() const noexcept {
            return ranges::subrange(mElementVector.template begin<T>(), mElementVector.template end<T>());
        }

        template<typename T>
        [[nodiscard]] auto elementsMutable() noexcept {
            return ranges::subrange(mElementVector.template begin<T>(), mElementVector.template end<T>());
        }

        [[nodiscard]] auto typeErasedElements() const noexcept {
            return ranges::subrange(mElementVector.cbegin(), mElementVector.cend());
        }

    private:
        std::vector<SparseIndex> mSparseVector;
        std::vector<SparseIndex> mDenseVector;
        TypeErasedVector mElementVector;
    };

}// namespace c2k