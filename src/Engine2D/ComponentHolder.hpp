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
#include <functional>

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
    void addComponent(Entity entity, const Component& component) noexcept;

    template<typename Component>
    [[nodiscard]] bool hasComponent(Entity entity) const noexcept;

    template<typename Component>
    [[nodiscard]] SparseSet<Component, Entity>& getComponentMutable() noexcept;

    template<typename Component>
    [[nodiscard]] const SparseSet<Component, Entity>& getComponent() const noexcept;

    template<typename FirstComponent, typename... Components>
    [[nodiscard]] auto getComponents() noexcept;

private:
    template<typename Component>
    std::size_t growIfNecessaryAndGetTypeIdentifier() noexcept;

private:
    std::vector<void*> mAddresses;
    std::vector<void (*)(void*)> mDestructors;
    std::size_t mSetSize;
};

template<std::unsigned_integral Entity>
template<typename Component>
void ComponentHolder<Entity>::create() noexcept {
    growIfNecessaryAndGetTypeIdentifier<Component>();
}

template<std::unsigned_integral Entity>
template<typename Component>
SparseSet<Component, Entity>& ComponentHolder<Entity>::getComponentMutable() noexcept {
    using SetType = SparseSet<Component, Entity>;
    const auto typeIdentifier = growIfNecessaryAndGetTypeIdentifier<Component>();
    return *static_cast<SetType*>(mAddresses[typeIdentifier]);
}

template<std::unsigned_integral Entity>
template<typename Component>
const SparseSet<Component, Entity>& ComponentHolder<Entity>::getComponent() const noexcept {
    using SetType = SparseSet<Component, Entity>;
    const auto typeIdentifier = TypeIdentifier::get<Component>();
    assert(typeIdentifier < mAddresses.size());
    return *static_cast<SetType*>(mAddresses[typeIdentifier]);
}

template<std::unsigned_integral Entity>
template<typename FirstComponent, typename... Components>
[[nodiscard]] auto ComponentHolder<Entity>::getComponents() noexcept {
    return getComponentMutable<FirstComponent>().mutableZipView() |
           ranges::views::filter([this](auto tuple) { return (hasComponent<Components>(std::get<0>(tuple)) && ...); }) |
           ranges::views::transform([this](auto tuple) {
               return std::make_tuple(std::get<0>(tuple), std::get<1>(tuple),
                                      getComponentMutable<Components>().getComponentMutable(std::get<0>(tuple))...);
           });
}

template<std::unsigned_integral Entity>
template<typename Component>
void ComponentHolder<Entity>::addComponent(Entity entity, const Component& component) noexcept {
    getComponentMutable<Component>().addComponent(entity, std::move(component));
}

template<std::unsigned_integral Entity>
template<typename Component>
[[nodiscard]] bool ComponentHolder<Entity>::hasComponent(Entity entity) const noexcept {
    return getComponent<Component>().hasComponent(entity);
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
    const bool needsResizing = typeIdentifier >= mDestructors.size();
    if (!needsResizing && mAddresses[typeIdentifier] != nullptr) {
        return typeIdentifier;
    }
    if (needsResizing) {
        mAddresses.resize(typeIdentifier + 1);
        mDestructors.resize(typeIdentifier + 1);
    }
    mAddresses[typeIdentifier] = new SetType{ static_cast<Entity>(mSetSize) };
    mDestructors[typeIdentifier] = [](void* address) { delete static_cast<SetType*>(address); };
    return typeIdentifier;
}