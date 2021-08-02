//
// Created by coder2k on 26.07.2021.
//

#pragma once

#include "ComponentHolder.hpp"
#include "strong_type/strong_type.hpp"
#include <gsl/gsl>
#include <concepts>
#include <numeric>
#include <limits>

template<std::unsigned_integral Entity, std::size_t identifierBits = sizeof(Entity) * 8 * 20 / 32>
class Registry final {
public:
    explicit Registry(std::size_t initialEntityCapacity) : mComponentHolder{ initialEntityCapacity } {
        mEntities.reserve(initialEntityCapacity);
    }

    template<typename Component>
    void addComponent(Entity entity, const Component& component) noexcept;

    template<typename Component>
    [[nodiscard]] bool hasComponent(Entity entity) noexcept {
        return mComponentHolder.template has<Component>(value_of(getIdentifierBitsFromEntity(entity)));
    }

    template<typename... Components>
    [[nodiscard]] auto getComponentsMutable() noexcept;

    template<typename... Components>
    [[nodiscard]] auto getComponents() const noexcept;

    [[nodiscard]] Entity createEntity() noexcept;
    void destroyEntity(Entity entity) noexcept;
    [[nodiscard]] bool isEntityAlive(Entity entity) const noexcept;
    [[nodiscard]] std::size_t numEntities() const noexcept {
        return mEntities.size();
    }
    [[nodiscard]] std::size_t numEntitiesAlive() const noexcept {
        return numEntities() - numEntitiesDead();
    }
    [[nodiscard]] std::size_t numEntitiesDead() const noexcept {
        return mNumRecyclableEntities;
    }

private:
    using Generation = strong::type<Entity, struct generation_, strong::equality, strong::incrementable>;
    using Identifier = strong::type<Entity, struct identifier_, strong::equality>;

private:
    [[nodiscard]] static Identifier getIdentifierBitsFromEntity(Entity entity) noexcept;
    [[nodiscard]] static Generation getGenerationBitsFromEntity(Entity entity) noexcept;
    [[nodiscard]] static Entity entityFromIdentifierAndGeneration(Identifier identifier,
                                                                  Generation generation) noexcept;
    static void swapIdentifiers(Entity& entity1, Entity& entity2) noexcept;
    static void increaseGeneration(Entity& entity) noexcept;
    [[nodiscard]] static std::size_t getIndexFromEntity(Entity entity) noexcept;

private:
    static constexpr std::size_t generationBits = sizeof(Entity) * 8 - identifierBits;
    static constexpr Entity generationMask = std::numeric_limits<Entity>::max() >> identifierBits;
    static constexpr Entity identifierMask = std::numeric_limits<Entity>::max() << generationBits;
    ComponentHolder<Entity> mComponentHolder;
    std::vector<Entity> mEntities;
    std::size_t mNumRecyclableEntities{ 0 };
    Entity mNextRecyclableEntity{ invalidEntity<Entity> };
};

template<std::unsigned_integral Entity, std::size_t IdentifierBits>
template<typename Component>
void Registry<Entity, IdentifierBits>::addComponent(Entity entity, const Component& component) noexcept {
    mComponentHolder.template attach<Component>(value_of(getIdentifierBitsFromEntity(entity)), component);
}

template<std::unsigned_integral Entity, std::size_t IdentifierBits>
template<typename... Components>
[[nodiscard]] auto Registry<Entity, IdentifierBits>::getComponentsMutable() noexcept {
    return mComponentHolder.template getMutable<Components...>();
}

template<std::unsigned_integral Entity, std::size_t IdentifierBits>
template<typename... Components>
[[nodiscard]] auto Registry<Entity, IdentifierBits>::getComponents() const noexcept {
    return mComponentHolder.template get<Components...>();
}

template<std::unsigned_integral Entity, std::size_t IdentifierBits>
[[nodiscard]] typename Registry<Entity, IdentifierBits>::Identifier
Registry<Entity, IdentifierBits>::getIdentifierBitsFromEntity(Entity entity) noexcept {
    return Identifier{ (entity & identifierMask) >> generationBits };
}

template<std::unsigned_integral Entity, std::size_t IdentifierBits>
[[nodiscard]] typename Registry<Entity, IdentifierBits>::Generation
Registry<Entity, IdentifierBits>::getGenerationBitsFromEntity(Entity entity) noexcept {
    return Generation{ entity & generationMask };
}

template<std::unsigned_integral Entity, std::size_t IdentifierBits>
[[nodiscard]] Entity Registry<Entity, IdentifierBits>::entityFromIdentifierAndGeneration(
        Identifier identifier,
        Generation generation) noexcept {
    return Entity{ (value_of(identifier) << generationBits) | value_of(generation) };
}

template<std::unsigned_integral Entity, std::size_t IdentifierBits>
void Registry<Entity, IdentifierBits>::swapIdentifiers(Entity& entity1, Entity& entity2) noexcept {
    const auto temp = entityFromIdentifierAndGeneration(getIdentifierBitsFromEntity(entity1),
                                                        getGenerationBitsFromEntity(entity2));
    entity1 = entityFromIdentifierAndGeneration(getIdentifierBitsFromEntity(entity2),
                                                getGenerationBitsFromEntity(entity1));
    entity2 = temp;
}

template<std::unsigned_integral Entity, std::size_t IdentifierBits>
void Registry<Entity, IdentifierBits>::increaseGeneration(Entity& entity) noexcept {
    entity = entityFromIdentifierAndGeneration(getIdentifierBitsFromEntity(entity),
                                               ++getGenerationBitsFromEntity(entity));
}

template<std::unsigned_integral Entity, std::size_t IdentifierBits>
[[nodiscard]] std::size_t Registry<Entity, IdentifierBits>::getIndexFromEntity(Entity entity) noexcept {
    return static_cast<std::size_t>(value_of(getIdentifierBitsFromEntity(entity)));
}

template<std::unsigned_integral Entity, std::size_t IdentifierBits>
[[nodiscard]] Entity Registry<Entity, IdentifierBits>::createEntity() noexcept {
    if (mNumRecyclableEntities > 0) {
        const auto index = getIndexFromEntity(mNextRecyclableEntity);
        swapIdentifiers(mEntities[index], mNextRecyclableEntity);
        --mNumRecyclableEntities;
        assert(static_cast<std::size_t>(value_of(getIdentifierBitsFromEntity(mEntities[index]))) == index);
        return mEntities[index];
    } else {
        mEntities.push_back(entityFromIdentifierAndGeneration(Identifier{ gsl::narrow_cast<Entity>(mEntities.size()) },
                                                              Generation{ 0 }));
        // TODO: add option to deny resizing
        if (mComponentHolder.size() < mEntities.size()) {
            mComponentHolder.resize(mEntities.size());
        }
        return mEntities.back();
    }
}

template<std::unsigned_integral Entity, std::size_t IdentifierBits>
void Registry<Entity, IdentifierBits>::destroyEntity(Entity entity) noexcept {
    // TODO: assert that entity is alive
    const auto index = getIndexFromEntity(entity);
    increaseGeneration(mEntities[index]);
    swapIdentifiers(mNextRecyclableEntity, mEntities[index]);
    ++mNumRecyclableEntities;
}

template<std::unsigned_integral Entity, std::size_t IdentifierBits>
[[nodiscard]] bool Registry<Entity, IdentifierBits>::isEntityAlive(Entity entity) const noexcept {
    const auto index = getIndexFromEntity(entity);
    assert(mEntities.size() > index);
    return mEntities[index] == entity;
}