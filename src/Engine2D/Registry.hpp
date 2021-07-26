//
// Created by coder2k on 26.07.2021.
//

#pragma once

#include "ComponentHolder.hpp"
#include <concepts>

template<std::unsigned_integral Entity>
class Registry final {
public:
    explicit Registry(std::size_t initialEntityCapacity) : mComponentHolder{ initialEntityCapacity } { }

    template<typename Component>
    void addComponent(Entity entity, const Component& component) noexcept {
        mComponentHolder.template addComponent<Component>(entity, component);
    }

    template<typename... Components>
    [[nodiscard]]auto getComponents() noexcept {
        return mComponentHolder.template getComponents<Components...>();
    }

private:
    ComponentHolder<Entity> mComponentHolder;
};