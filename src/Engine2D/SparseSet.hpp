//
// Created by coder2k on 18.07.2021.
//

#pragma once

#include "pch.hpp"// TODO: delete include "pch.hpp" in all files since CMake force includes it
#include "TypeErasedVector.hpp"
#include "Tag.hpp"
#include <list>

namespace c2k {

    template<std::unsigned_integral SparseIndex, SparseIndex invalidIndex>
    class SparseSet final {
    public:
        template<typename T>
        explicit SparseSet(Tag<T>,
                           SparseIndex initialSetSize,
                           SparseIndex initialElementCapacity = SparseIndex{ 0 }) noexcept
            : mSparseVector{ initialSetSize, invalidIndex },
              mElementVector{ TypeErasedVector::forType<T>() } {
            mDenseVector.reserve(initialElementCapacity);
            mElementVector.reserve(initialElementCapacity);
        }

        template<typename Component>
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
            mElementVector.swapElements(denseIndex, mElementVector.size() - 1);
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

        template<typename T>
        [[nodiscard]] const T& get(SparseIndex index) const noexcept {
            assert(has(index) && "The given index doesn't have an instance of this element.");
            return mElementVector.get<T>(mSparseVector[index]);
        }

        template<typename T>
        [[nodiscard]] T& getMutable(SparseIndex index) noexcept {
            assert(has(index) && "The given index doesn't have an instance of this element.");
            return mElementVector.get<T>(mSparseVector[index]);
        }
        [[nodiscard]] auto indices() const noexcept {
            return mDenseVector | ranges::views::all;
        }

        /*template<std::assignable_from T>
        struct S {

        };*/

        template<typename T>
        [[nodiscard]] auto elements() const noexcept {
            const auto beginIterator = mElementVector.template begin<T>();
            const auto endIterator = mElementVector.template end<T>();
            /*S<decltype(beginIterator)> s{};
            static_assert(std::is_object_v<decltype(beginIterator)>);
            static_assert(std::is_lvalue_reference<decltype(beginIterator)>);

            std::list<int> numbers{ 1, 2, 3 };*/
            //return ranges::subrange(numbers.cbegin(), numbers.cend());
            return ranges::subrange(beginIterator, endIterator) | ranges::views::all;
        }

        template<typename T>
        [[nodiscard]] auto elementsMutable() noexcept {
            return ranges::subrange(mElementVector.template begin<T>(), mElementVector.template end<T>()) |
                   ranges::views::all;
        }

    private:
        std::vector<SparseIndex> mSparseVector;
        std::vector<SparseIndex> mDenseVector;
        TypeErasedVector mElementVector;
    };

}// namespace c2k