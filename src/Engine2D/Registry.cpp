//
// Created by coder2k on 06.09.2021.
//

#include "Registry.hpp"

namespace c2k {

    SystemHolder::SystemHolder(Registry& registry) : mRegistry{ registry } { }

    SystemHolder::~SystemHolder() {
        using ranges::views::zip;
        for (auto&& [systemContext, destructor] : zip(mSystemContexts, mDestructors)) {
            assert(destructor && "Destructor must not be nullptr");
            destructor(systemContext.address);
        }
    }

    void SystemHolder::run() noexcept {
        using ranges::views::zip;
        for (const auto& systemContext : mSystemContexts) {
            systemContext.setupFunction(systemContext.address);
            systemContext.forEachFunction(systemContext.address, this);
            systemContext.finalizeFunction(systemContext.address);
        }
    }


}// namespace c2k