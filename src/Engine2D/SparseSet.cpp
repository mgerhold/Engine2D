//
// Created by coder2k on 07.09.2021.
//
#include "SparseSet.hpp"

namespace c2k {

    void SparseSet::remove(Entity index) noexcept {
        using std::swap;
        assert(has(index) && "The given index doesn't have an instance of this element.");
        const auto denseIndex = mSparseVector[index];
        mSparseVector[index] = invalidEntity;
        mSparseVector[mDenseVector.back()] = denseIndex;
        mDenseVector[denseIndex] = mDenseVector.back();
        mDenseVector.resize(mDenseVector.size() - 1);
        mElementVector.swapElements(denseIndex, mElementVector.size() - 1);
        mElementVector.resize(mElementVector.size() - 1);
        assert(mDenseVector.size() == mElementVector.size());
    }

}// namespace c2k
