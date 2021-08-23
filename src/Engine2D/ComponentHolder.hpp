//
// Created by coder2k on 19.07.2021.
//

#pragma once

#include "Entity.hpp"
#include "SparseSet.hpp"

namespace c2k {

    template<std::unsigned_integral SparseIndex, typename TypeIdentifier>
    class ComponentHolder final {
    public:
        explicit ComponentHolder(std::size_t initialSetSize) noexcept : mSetSize{ initialSetSize } { }
        ~ComponentHolder() {
            for (const auto pointerToSet : mSparseSets) {
                delete pointerToSet;
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
            for (const auto pointerToSet : mSparseSets) {
                if (pointerToSet != nullptr) {
                    pointerToSet->resize(size);
                }
            }
            mSetSize = size;
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
            auto indices = getComponent<FirstComponent>().indices();
            auto elements = getComponentMutable<FirstComponent>().template elementsMutable<FirstComponent>();
            return zip(indices, elements);
            /*getComponent<FirstComponent>().indices(),
                       getComponentMutable<FirstComponent>().template elementsMutable<FirstComponent>() |
                   // tuple is marked as maybe_unused in the next line because MSVC reports a warning
                   filter([this]([[maybe_unused]] auto&& tuple) {
                       return (has<Components>(std::get<0>(tuple)) && ...);
                   }) |
                   transform([this](auto&& tuple) {
                       return std::forward_as_tuple(std::get<0>(tuple), std::get<1>(tuple),
                                                    getComponentMutable<Components>().template getMutable<Components>(
                                                            std::get<0>(tuple))...);
                   });*/
        }

        template<typename FirstComponent, typename... Components>
        [[nodiscard]] auto get() const noexcept {
            using ranges::views::filter, ranges::views::transform, ranges::views::zip;
            return zip(getComponent<FirstComponent>().indices(),
                       getComponent<FirstComponent>().template elements<FirstComponent>()) |
                   // tuple is marked as maybe_unused in the next line because MSVC reports a warning
                   filter([this]([[maybe_unused]] auto&& tuple) {
                       return (has<Components>(std::get<0>(tuple)) && ...);
                   }) |
                   transform([this](auto&& tuple) {
                       return std::forward_as_tuple(
                               std::get<0>(tuple), std::get<1>(tuple),
                               getComponent<Components>().template get<Components>(std::get<0>(tuple))...);
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
        template<typename Component>
        [[nodiscard]] std::size_t typeIdentifier() const noexcept {
            return TypeIdentifier::template get<Component>();
        }

    private:
        using ComponentSet = SparseSet<SparseIndex, invalidEntity<SparseIndex>>;

    private:
        template<typename Component>
        [[nodiscard]] ComponentSet& getComponentMutable() noexcept {
            const auto typeIdentifier = growIfNecessaryAndGetTypeIdentifier<Component>();
            assert(typeIdentifier < mSparseSets.size());
            return *mSparseSets[typeIdentifier];
        }
        template<typename Component>
        [[nodiscard]] const ComponentSet& getComponent() const noexcept {
            const auto typeIdentifier = TypeIdentifier::template get<Component>();
            assert(typeIdentifier < mSparseSets.size());
            return *mSparseSets[typeIdentifier];
        }
        template<typename Component>
        std::size_t growIfNecessaryAndGetTypeIdentifier() noexcept {
            using SetType = SparseSet<SparseIndex, invalidEntity<SparseIndex>>;
            const auto typeIdentifier = TypeIdentifier::template get<Component>();
            const bool needsResizing = typeIdentifier >= mSparseSets.size();
            if (!needsResizing && mSparseSets[typeIdentifier] != nullptr) {
                return typeIdentifier;
            }
            if (needsResizing) {
                mSparseSets.resize(typeIdentifier + 1, nullptr);
            }
            mSparseSets[typeIdentifier] = new SetType{ Tag<Component>{}, static_cast<SparseIndex>(mSetSize) };
            return typeIdentifier;
        }
        template<typename Component>
        [[nodiscard]] bool doesExist() const noexcept {
            const auto typeIdentifier = TypeIdentifier::template get<Component>();
            return typeIdentifier < mSparseSets.size();
        }

    private:
        std::vector<ComponentSet*> mSparseSets;
        std::size_t mSetSize;
    };

}// namespace c2k