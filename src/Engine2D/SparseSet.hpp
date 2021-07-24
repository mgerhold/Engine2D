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

template<typename Component, std::unsigned_integral Entity>
class SparseSet final {
public:
    using value_type = Component;

public:
    SparseSet(Entity initialSetSize, Entity initialComponentCapacity = 0) noexcept;

    [[nodiscard]] bool hasComponent(Entity entity) const noexcept;

    template<typename T>
    void addComponent(Entity entity, T&& component) noexcept;

    [[nodiscard]] const Component& getComponent(Entity entity) const noexcept;
    [[nodiscard]] Component& getComponentMutable(Entity entity) noexcept;
    void deleteComponent(Entity entity) noexcept;

    [[nodiscard]] std::size_t componentCount() const noexcept {
        return mComponentVector.size();
    }

    [[nodiscard]] auto mutableView() noexcept;
    [[nodiscard]] auto entityView() const noexcept;
    [[nodiscard]] auto componentView() const noexcept;

    template<typename Callable>
    void forEachComponent(Callable callable) noexcept;

    [[nodiscard]] auto zipView() const noexcept;
    [[nodiscard]] auto mutableZipView() noexcept;

    template<typename Callable>
    void forEachPair(Callable callable) noexcept;

private:
    static constexpr Entity InvalidEntity = std::numeric_limits<Entity>::max();

private:
    std::vector<Entity> mSparseVector;
    std::vector<Entity> mDenseVector;
    std::vector<Component> mComponentVector;
};

template<typename Component, std::unsigned_integral Entity>
SparseSet<Component, Entity>::SparseSet(Entity initialSetSize, Entity initialComponentCapacity) noexcept
    : mSparseVector(initialSetSize, InvalidEntity) {
    mDenseVector.reserve(initialComponentCapacity);
    mComponentVector.reserve(initialComponentCapacity);
}

template<typename Component, std::unsigned_integral Entity>
bool SparseSet<Component, Entity>::hasComponent(Entity entity) const noexcept {
    assert(entity < mSparseVector.size() && "Invalid entity id.");
    const auto denseIndex = mSparseVector[entity];
    return denseIndex < mDenseVector.size();
}

template<typename Component, std::unsigned_integral Entity>
template<typename T>
void SparseSet<Component, Entity>::addComponent(Entity entity, T&& component) noexcept {
    assert(entity < mSparseVector.size() && "Invalid entity id.");
    mSparseVector[entity] = static_cast<Entity>(mDenseVector.size());
    mDenseVector.push_back(entity);
    mComponentVector.push_back(std::forward<T>(component));
}

template<typename Component, std::unsigned_integral Entity>
const Component& SparseSet<Component, Entity>::getComponent(Entity entity) const noexcept {
    assert(hasComponent(entity) && "The given entity doesn't have an instance of this component.");
    return mComponentVector[mSparseVector[entity]];
}

template<typename Component, std::unsigned_integral Entity>
Component& SparseSet<Component, Entity>::getComponentMutable(Entity entity) noexcept {
    assert(hasComponent(entity) && "The given entity doesn't have an instance of this component.");
    return mComponentVector[mSparseVector[entity]];
}

template<typename Component, std::unsigned_integral Entity>
void SparseSet<Component, Entity>::deleteComponent(Entity entity) noexcept {
    using std::swap;
    assert(hasComponent(entity) && "The given entity doesn't have an instance of this component.");
    const auto denseIndex = mSparseVector[entity];
    mSparseVector[entity] = InvalidEntity;
    mSparseVector[mDenseVector.back()] = denseIndex;
    mDenseVector[denseIndex] = mDenseVector.back();
    mDenseVector.resize(mDenseVector.size() - 1);
    mComponentVector[denseIndex] = std::move(mComponentVector.back());
    mComponentVector.resize(mComponentVector.size() - 1);
    assert(mDenseVector.size() == mComponentVector.size());
}

template<typename Component, std::unsigned_integral Entity>
[[nodiscard]] auto SparseSet<Component, Entity>::mutableView() noexcept {
    return mComponentVector | ranges::views::all;
}

template<typename Component, std::unsigned_integral Entity>
auto SparseSet<Component, Entity>::entityView() const noexcept {
    return mDenseVector | ranges::views::all;
}
template<typename Component, std::unsigned_integral Entity>
auto SparseSet<Component, Entity>::componentView() const noexcept {
    return mComponentVector | ranges::views::all;
}

template<typename Component, std::unsigned_integral Entity>
template<typename Callable>
void SparseSet<Component, Entity>::forEachComponent(Callable callable) noexcept {
    for (auto& component : mComponentVector) {
        callable(component);
    }
}

template<typename Component, std::unsigned_integral Entity>
auto SparseSet<Component, Entity>::zipView() const noexcept {
    return ranges::views::zip(mDenseVector, mComponentVector);
}

template<typename Component, std::unsigned_integral Entity>
auto SparseSet<Component, Entity>::mutableZipView() noexcept {
    const auto& entities = mDenseVector;
    return ranges::views::zip(entities, mComponentVector);
}

template<typename Component, std::unsigned_integral Entity>
template<typename Callable>
void SparseSet<Component, Entity>::forEachPair(Callable callable) noexcept {
    assert(mDenseVector.size() == mComponentVector.size());
    const auto indexEndIt = mDenseVector.cend();
    auto entityIt = mDenseVector.cbegin();
    auto componentIt = mComponentVector.begin();
    for (; entityIt != indexEndIt; ++entityIt, ++componentIt) {
        callable(*entityIt, *componentIt);
    }
}