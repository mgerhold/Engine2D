//
// Created by coder2k on 09.08.2021.
//

#pragma once

#include "System.hpp"
#include "ComponentHolder.hpp"
#include <range/v3/all.hpp>
#include <functional>
#include <concepts>
#include <vector>
#include <cassert>

template<std::unsigned_integral Entity, typename TypeIdentifier, typename ComponentHolderType>
class SystemHolder final {
public:
    explicit SystemHolder(ComponentHolderType& componentHolder) : mComponentHolder{ componentHolder } { }

    ~SystemHolder() {
        using ranges::views::zip;
        for (auto&& [systemContext, destructor] : zip(mSystemContexts, mDestructors)) {
            assert(destructor && "Destructor must not be nullptr");
            destructor(systemContext.address);
        }
    }

    void run() noexcept {
        using ranges::views::zip;
        for (const auto& systemContext : mSystemContexts) {
            systemContext.setupFunction(systemContext.address);
            systemContext.forEachFunction(systemContext.address, this);
            systemContext.finalizeFunction(systemContext.address);
        }
    }

    template<typename... Components>
    void emplace(auto&& setup, auto&& forEach, auto&& finalize) noexcept {
        using SystemType = System<Entity, Components...>;
        const auto typeIdentifier = TypeIdentifier::template get<SystemType>();
        const bool needsResizing = typeIdentifier >= mSystemContexts.size();
        assert((needsResizing || mSystemContexts[typeIdentifier].address == nullptr) &&
               "Index of this System is already taken");
        if (needsResizing) {
            mSystemContexts.resize(typeIdentifier + 1);
            mDestructors.resize(typeIdentifier + 1);
        }
        mSystemContexts[typeIdentifier] = SystemContext{
            .address{ new SystemType{ std::forward<decltype(setup)>(setup), std::forward<decltype(forEach)>(forEach),
                                      std::forward<decltype(finalize)>(finalize) } },
            .setupFunction{ [](void* address) { static_cast<SystemType*>(address)->setup(); } },
            .forEachFunction{ [](void* address, SystemHolder* self) {
                static_cast<SystemType*>(address)->forEach(
                        self->mComponentHolder.template getMutable<std::remove_cvref_t<Components>...>());
            } },
            .finalizeFunction{ [](void* address) { static_cast<SystemType*>(address)->finalize(); } }
        };
        mDestructors[typeIdentifier] = [](void* address) { delete static_cast<SystemType*>(address); };
    }

private:
    struct SystemContext {
        void* address{ nullptr };
        void (*setupFunction)(void*){ nullptr };
        void (*forEachFunction)(void*, SystemHolder*){ nullptr };
        void (*finalizeFunction)(void*){ nullptr };
    };

private:
    std::vector<SystemContext> mSystemContexts;
    std::vector<void (*)(void*)> mDestructors;
    ComponentHolderType& mComponentHolder;
};