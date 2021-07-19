//
// Created by coder2k on 19.07.2021.
//

#pragma once

#include "SparseSet.hpp"
#include "TypeIdentifier.hpp"
#include <range/v3/all.hpp>
#include <concepts>
#include <vector>
#include <cassert>

template<std::unsigned_integral Entity>
class ComponentHolder final {
public:
    explicit ComponentHolder(std::size_t initialSetSize) noexcept : mSetSize{ initialSetSize } {
        assert(initialSetSize > 0 && "Initial set size must be greater than zero.");
    }

    ~ComponentHolder();

    template<typename Component>
    void create() noexcept;

    template<typename Component>
    [[nodiscard]] SparseSet<Component, Entity>& getMutable() noexcept;

    template<typename Component>
    [[nodiscard]] const SparseSet<Component, Entity>& get() const noexcept;

private:
    template<typename Component>
    std::size_t growIfNecessaryAndGetTypeIdentifier() noexcept;

private:
    std::vector<void*> mAddresses;
    std::vector<void(*)(void*)> mDestructors;
    std::size_t mSetSize;
};

template<std::unsigned_integral Entity>
template<typename Component>
void ComponentHolder<Entity>::create() noexcept {
    growIfNecessaryAndGetTypeIdentifier<Component>();
}

template<std::unsigned_integral Entity>
template<typename Component>
SparseSet<Component, Entity>& ComponentHolder<Entity>::getMutable() noexcept {
    using SetType = SparseSet<Component, Entity>;
    const auto typeIdentifier = growIfNecessaryAndGetTypeIdentifier<Component>();
    return *static_cast<SetType*>(mAddresses[typeIdentifier]);
}

template<std::unsigned_integral Entity>
template<typename Component>
const SparseSet<Component, Entity>& ComponentHolder<Entity>::get() const noexcept {
    using SetType = SparseSet<Component, Entity>;
    const auto typeIdentifier = TypeIdentifier::get<Component>();
    assert(typeIdentifier < mAddresses.size());
    return *static_cast<SetType*>(mAddresses[typeIdentifier]);
}

template<std::unsigned_integral Entity>
ComponentHolder<Entity>::~ComponentHolder() {
    for (const auto& [address, destructor] : ranges::views::zip(mAddresses, mDestructors)) {
        destructor(address);
    }
}

template<std::unsigned_integral Entity>
template<typename Component>
std::size_t ComponentHolder<Entity>::growIfNecessaryAndGetTypeIdentifier() noexcept {
    using SetType = SparseSet<Component, Entity>;
    const auto typeIdentifier = TypeIdentifier::get<Component>();
    if (typeIdentifier >= mDestructors.size()) {
        mAddresses.resize(typeIdentifier + 1);
        mDestructors.resize(typeIdentifier + 1);
        mAddresses[typeIdentifier] = new SetType{ static_cast<Entity>(mSetSize) };
        mDestructors[typeIdentifier] = [](void* address) { delete static_cast<SetType*>(address); };
    }
    return typeIdentifier;
}