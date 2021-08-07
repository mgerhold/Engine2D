//
// Created by coder2k on 19.07.2021.
//

#pragma once

#include "Entity.hpp"
#include "SparseSet.hpp"
#include "TypeIdentifier.hpp"
#include <spdlog/spdlog.h>
#include <range/v3/all.hpp>
#include <concepts>
#include <vector>
#include <cassert>
#include <functional>

template<std::unsigned_integral SparseIndex>
class ComponentHolder final {
public:
    explicit ComponentHolder(std::size_t initialSetSize) noexcept : mSetSize{ initialSetSize } { }
    ~ComponentHolder() {
        for (const auto& [address, destructor] : ranges::views::zip(mAddresses, mDestructors)) {
            destructor(address);
        }
    }

    template<typename Component>
    void init() noexcept {
        growIfNecessaryAndGetTypeIdentifier<Component>();
    }
    template<typename Component>
    void attach(SparseIndex entity, const Component& component) noexcept {
        getComponentMutable<Component>().add(entity, std::move(component));
    }
    void resize(std::size_t size) noexcept {
        using ranges::views::zip;
        assert(size >= mSetSize);
        for (auto&& [resizeFunction, address] : zip(mResizeFunctions, mAddresses)) {
            resizeFunction(address, size);
        }
        mSetSize = size;
        spdlog::info("New size: {}", mSetSize);
    }
    [[nodiscard]] std::size_t size() const noexcept {
        return mSetSize;
    }

    template<typename Component>
    [[nodiscard]] bool has(SparseIndex entity) const noexcept {
        return doesExist<Component>() && getComponent<Component>().has(entity);
    }
    template<typename FirstComponent, typename... Components>
    [[nodiscard]] auto getMutable() noexcept {
        using ranges::views::filter, ranges::views::transform, ranges::views::zip;
        return zip(getComponent<FirstComponent>().indices(), getComponentMutable<FirstComponent>().elementsMutable()) |
               // tuple is marked as maybe_unused in the next line because MSVC reports a warning
               filter([this]([[maybe_unused]] auto&& tuple) { return (has<Components>(std::get<0>(tuple)) && ...); }) |
               transform([this](auto&& tuple) {
                   return std::forward_as_tuple(std::get<0>(tuple), std::get<1>(tuple),
                                                getComponentMutable<Components>().getMutable(std::get<0>(tuple))...);
               });
    }
    template<typename FirstComponent, typename... Components>
    [[nodiscard]] auto get() const noexcept {
        using ranges::views::filter, ranges::views::transform, ranges::views::zip;
        return zip(getComponent<FirstComponent>().indices(), getComponent<FirstComponent>().elements()) |
               // tuple is marked as maybe_unused in the next line because MSVC reports a warning
               filter([this]([[maybe_unused]] auto&& tuple) { return (has<Components>(std::get<0>(tuple)) && ...); }) |
               transform([this](auto&& tuple) {
                   return std::forward_as_tuple(std::get<0>(tuple), std::get<1>(tuple),
                                                getComponent<Components>().get(std::get<0>(tuple))...);
               });
    }
    template<typename Component>
    [[nodiscard]] Component& getMutable(SparseIndex entity) noexcept {
        return getComponentMutable<Component>().getMutable(entity);
    }
    template<typename Component>
    [[nodiscard]] const Component& get(SparseIndex entity) const noexcept {
        return getComponent<Component>().get(entity);
    }

private:
    template<typename Component>
    using ComponentSet = SparseSet<Component, SparseIndex, invalidEntity<SparseIndex>>;

private:
    template<typename Component>
    [[nodiscard]] decltype(auto) getComponentMutable() noexcept {
        const auto typeIdentifier = growIfNecessaryAndGetTypeIdentifier<Component>();
        return *static_cast<ComponentSet<Component>*>(mAddresses[typeIdentifier]);
    }
    template<typename Component>
    [[nodiscard]] decltype(auto) getComponent() const noexcept {
        const auto typeIdentifier = TypeIdentifier::get<Component>();
        assert(typeIdentifier < mAddresses.size());
        return *static_cast<ComponentSet<Component>*>(mAddresses[typeIdentifier]);
    }
    template<typename Component>
    std::size_t growIfNecessaryAndGetTypeIdentifier() noexcept {
        using SetType = SparseSet<Component, SparseIndex, invalidEntity<SparseIndex>>;
        const auto typeIdentifier = TypeIdentifier::get<Component>();
        const bool needsResizing = typeIdentifier >= mDestructors.size();
        if (!needsResizing && mAddresses[typeIdentifier] != nullptr) {
            return typeIdentifier;
        }
        if (needsResizing) {
            mAddresses.resize(typeIdentifier + 1);
            mResizeFunctions.resize(typeIdentifier + 1);
            mDestructors.resize(typeIdentifier + 1);
        }
        spdlog::info("Resizing component holder, new size: {}", mAddresses.size());
        mAddresses[typeIdentifier] = new SetType{ static_cast<SparseIndex>(mSetSize) };
        mResizeFunctions[typeIdentifier] = [](void* address, std::size_t size) {
            static_cast<SetType*>(address)->resize(size);
        };
        mDestructors[typeIdentifier] = [](void* address) { delete static_cast<SetType*>(address); };
        return typeIdentifier;
    }
    template<typename Component>
    [[nodiscard]] bool doesExist() const noexcept {
        const auto typeIdentifier = TypeIdentifier::get<Component>();
        return typeIdentifier < mAddresses.size();
    }

private:
    std::vector<void*> mAddresses;
    std::vector<void (*)(void*)> mDestructors;
    std::vector<void (*)(void*, std::size_t)> mResizeFunctions;
    std::size_t mSetSize;
};