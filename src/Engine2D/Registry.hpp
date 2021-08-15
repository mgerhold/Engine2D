//
// Created by coder2k on 26.07.2021.
//

#pragma once

#include "ComponentHolder.hpp"
#include "optional/optional.hpp"
#include "Entity.hpp"
#include "TypeIdentifier.hpp"
#include "SystemHolder.hpp"
#include <gsl/gsl>
#include <functional>
#include <concepts>
#include <numeric>
#include <limits>

template<std::unsigned_integral Entity, std::size_t identifierBits = sizeof(Entity) * 8 * 20 / 32>
class RegistryBase final {
public:
    using EntityType = Entity;

public:
    explicit RegistryBase(std::size_t initialEntityCapacity = 0)
        : mComponentHolder{ initialEntityCapacity },
          mSystemHolder{ mComponentHolder } {
        mEntities.reserve(initialEntityCapacity);
    }

    template<typename Component>
    void attachComponent(Entity entity, const Component& component) noexcept {
        mComponentHolder.template attach<Component>(getIdentifierBitsFromEntity(entity), component);
    }
    template<typename Component>
    [[nodiscard]] bool hasComponent(Entity entity) const noexcept {
        return mComponentHolder.template has<Component>(getIdentifierBitsFromEntity(entity));
    }
    template<typename Component>
    [[nodiscard]] tl::optional<const Component&> component(Entity entity) const noexcept {
        if (!hasComponent<Component>(entity)) {
            return {};
        }
        return mComponentHolder.template get<Component>(getIdentifierBitsFromEntity(entity));
    }
    template<typename Component>
    [[nodiscard]] tl::optional<Component&> componentMutable(Entity entity) noexcept {
        if (!hasComponent<Component>(entity)) {
            return {};
        }
        return mComponentHolder.template getMutable<Component>(getIdentifierBitsFromEntity(entity));
    }
    template<typename... Components>
    [[nodiscard]] auto componentsMutable() noexcept {
        using ranges::views::transform;
        return mComponentHolder.template getMutable<Components...>() | transform([this](auto&& tuple) {
                   return std::apply(
                           [this](auto&& identifier, auto&&... rest) {
                               return std::forward_as_tuple(mEntities[identifier], rest...);
                           },
                           tuple);
               });
    }
    template<typename... Components>
    [[nodiscard]] auto components() const noexcept {
        using ranges::views::transform;
        return mComponentHolder.template get<Components...>() | transform([this](auto&& tuple) {
                   return std::apply(
                           [this](auto&& identifier, auto&&... rest) {
                               return std::forward_as_tuple(mEntities[identifier], rest...);
                           },
                           tuple);
               });
    }
    [[nodiscard]] Entity createEntity() noexcept {
        if (mNumRecyclableEntities > 0) {
            const auto index = getIndexFromEntity(mNextRecyclableEntity);
            swapIdentifiers(mEntities[index], mNextRecyclableEntity);
            --mNumRecyclableEntities;
            assert(static_cast<std::size_t>(getIdentifierBitsFromEntity(mEntities[index])) == index);
            return mEntities[index];
        } else {
            mEntities.push_back(entityFromIdentifierAndGeneration(
                    Identifier{ gsl::narrow_cast<Entity>(mEntities.size()) }, Generation{ 0 }));
            // TODO: add option to deny resizing
            if (mComponentHolder.size() < mEntities.size()) {
                mComponentHolder.resize(mEntities.size());
            }
            return mEntities.back();
        }
    }
    void destroyEntity(Entity entity) noexcept {
        assert(isEntityAlive(entity) && "The entity to remove must be alive.");
        const auto index = getIndexFromEntity(entity);
        increaseGeneration(mEntities[index]);
        swapIdentifiers(mNextRecyclableEntity, mEntities[index]);
        ++mNumRecyclableEntities;
    }
    [[nodiscard]] bool isEntityAlive(Entity entity) const noexcept {
        const auto index = getIndexFromEntity(entity);
        assert(mEntities.size() > index);
        return mEntities[index] == entity;
    }
    [[nodiscard]] std::size_t numEntities() const noexcept {
        return mEntities.size();
    }
    [[nodiscard]] std::size_t numEntitiesAlive() const noexcept {
        return numEntities() - numEntitiesDead();
    }
    [[nodiscard]] std::size_t numEntitiesDead() const noexcept {
        return mNumRecyclableEntities;
    }
    template<typename Type>
    [[nodiscard]] std::size_t typeIdentifier() const noexcept {
        return mComponentHolder.template typeIdentifier<Type>();
    }
    template<typename... Components, typename SetupFunction, typename ForEachFunction, typename FinalizeFunction>
    void emplaceSystem(SetupFunction&& setup, ForEachFunction&& forEach, FinalizeFunction&& finalize) noexcept {
        mSystemHolder.template emplace<Components...>(std::forward<SetupFunction>(setup),
                                                      std::forward<ForEachFunction>(forEach),
                                                      std::forward<FinalizeFunction>(finalize));
    }
    void runSystems() noexcept {
        mSystemHolder.run();
    }

private:
    using Generation = Entity;
    using Identifier = Entity;

private:
    [[nodiscard]] static Identifier getIdentifierBitsFromEntity(Entity entity) noexcept {
        return Identifier{ (entity & identifierMask) >> generationBits };
    }
    [[nodiscard]] static Generation getGenerationBitsFromEntity(Entity entity) noexcept {
        return Generation{ entity & generationMask };
    }
    [[nodiscard]] static Entity entityFromIdentifierAndGeneration(Identifier identifier,
                                                                  Generation generation) noexcept {
        return Entity{ (identifier << generationBits) | generation };
    }
    static void swapIdentifiers(Entity& entity1, Entity& entity2) noexcept {
        const auto temp = entityFromIdentifierAndGeneration(getIdentifierBitsFromEntity(entity1),
                                                            getGenerationBitsFromEntity(entity2));
        entity1 = entityFromIdentifierAndGeneration(getIdentifierBitsFromEntity(entity2),
                                                    getGenerationBitsFromEntity(entity1));
        entity2 = temp;
    }
    static void increaseGeneration(Entity& entity) noexcept {
        entity = entityFromIdentifierAndGeneration(getIdentifierBitsFromEntity(entity),
                                                   getGenerationBitsFromEntity(entity) + 1);
    }
    [[nodiscard]] static std::size_t getIndexFromEntity(Entity entity) noexcept {
        return static_cast<std::size_t>(getIdentifierBitsFromEntity(entity));
    }

private:
    static constexpr std::size_t generationBits = sizeof(Entity) * 8 - identifierBits;
    static constexpr Entity generationMask = std::numeric_limits<Entity>::max() >> identifierBits;
    static constexpr Entity identifierMask = std::numeric_limits<Entity>::max() << generationBits;
    ComponentHolder<Identifier, TypeIdentifier<struct componentTypeIdentifier>> mComponentHolder;
    SystemHolder<Entity, TypeIdentifier<struct systemTypeIdentifier>, decltype(mComponentHolder)> mSystemHolder;
    std::vector<Entity> mEntities;
    std::size_t mNumRecyclableEntities{ 0 };
    Entity mNextRecyclableEntity{ invalidEntity<Entity> };
};

using Registry = RegistryBase<Entity>;