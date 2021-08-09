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
    SystemHolder(ComponentHolderType& componentHolder) : mComponentHolder{ componentHolder } { }

    ~SystemHolder() {
        using ranges::views::zip;
        for (auto&& [address, destructor] : zip(mAddresses, mDestructors)) {
            assert(destructor && "Destructor must not be nullptr");
            destructor(address);
        }
    }

    void run() noexcept {
        using ranges::views::zip;
        for (auto&& [address, setupFunction, forEachFunction, finalizeFunction] :
             zip(mAddresses, mSetupFunctions, mForEachFunctions, mFinalizeFunctions)) {
            setupFunction(address);
            forEachFunction(address, this);
            finalizeFunction(address);
        }
    }

    template<typename... Components>
    void emplace(std::function<void(void)> setup,
                 std::function<void(Entity, Components...)> forEach,
                 std::function<void(void)> finalize) noexcept {
        using SystemType = System<Entity, Components...>;
        const auto typeIdentifier = TypeIdentifier::template get<SystemType>();
        const bool needsResizing = typeIdentifier >= mAddresses.size();
        assert((needsResizing || mAddresses[typeIdentifier] == nullptr) && "Index of this System is already taken");
        if (needsResizing) {
            mAddresses.resize(typeIdentifier + 1);
            mSetupFunctions.resize(typeIdentifier + 1);
            mForEachFunctions.resize(typeIdentifier + 1);
            mFinalizeFunctions.resize(typeIdentifier + 1);
            mDestructors.resize(typeIdentifier + 1);
        }
        mAddresses[typeIdentifier] = new SystemType{ setup, forEach, finalize };
        mSetupFunctions[typeIdentifier] = [](void* address) { static_cast<SystemType*>(address)->setup(); };
        mForEachFunctions[typeIdentifier] = [](void* address, SystemHolder* self) {
            static_cast<SystemType*>(address)->forEach(
                    self->mComponentHolder.getMutable<std::remove_cvref_t<Components>...>());
        };
        mFinalizeFunctions[typeIdentifier] = [](void* address) { static_cast<SystemType*>(address)->finalize(); };
        mDestructors[typeIdentifier] = [](void* address) { delete static_cast<SystemType*>(address); };
    }

private:
    // TODO: create struct for all those except the dtor pointers because we almost
    //  always iterate over all of them atonce
    std::vector<void*> mAddresses;
    std::vector<void (*)(void*)> mSetupFunctions;
    std::vector<void (*)(void*, SystemHolder*)> mForEachFunctions;
    std::vector<void (*)(void*)> mFinalizeFunctions;
    std::vector<void (*)(void*)> mDestructors;
    ComponentHolderType& mComponentHolder;
};