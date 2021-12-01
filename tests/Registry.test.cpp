//
// Created by coder2k on 31.07.2021.
//

#include <Registry.hpp>
#include <TypeIdentifier.hpp>
#include <ComponentHolderPairIterator.hpp>
#include <Entity.hpp>
#include <gtest/gtest.h>
#include <range/v3/all.hpp>
#include <tl/optional.hpp>
#include <gsl/gsl>
#include <array>
#include <cstddef>
#include <iterator>
#include <new>
#include <tuple>
#include <utility>
#include <vector>

using namespace c2k;

struct Position {
    float x, y;
};

struct Velocity {
    float x, y;
};

TEST(RegistryComponentHolderGrowingFixture, TriggerTypeIdentifierCreation_UseInDifferentOrder) {
    Registry registry;
    const auto positionID = registry.typeIdentifier<Position>();
    ASSERT_EQ(positionID, 0);
    const auto velocityID = registry.typeIdentifier<Velocity>();
    ASSERT_EQ(velocityID, 1);
    const auto entity = registry.createEntity();
    registry.attachComponent(entity, Velocity{ 1.0f, 2.0f });
    registry.attachComponent(entity, Position{ 3.0f, 4.0f });
    const auto& optionalVelocity = registry.component<Velocity>(entity);
    ASSERT_TRUE(optionalVelocity);
    const auto& optionalPosition = registry.component<Position>(entity);
    ASSERT_TRUE(optionalPosition);
    ASSERT_EQ(optionalVelocity->x, 1.0f);
    ASSERT_EQ(optionalVelocity->y, 2.0f);
    ASSERT_EQ(optionalPosition->x, 3.0f);
    ASSERT_EQ(optionalPosition->y, 4.0f);
}

TEST(RegistryEntityCreationAndDestructionFixture, RecycleEntity_CheckAliveStatus) {
    Registry registry{ 100 };
    ASSERT_EQ(registry.numEntities(), 0);
    const std::array entities = { registry.createEntity(), registry.createEntity() };
    ASSERT_TRUE(registry.isEntityAlive(entities[0]));
    ASSERT_TRUE(registry.isEntityAlive(entities[1]));
    ASSERT_EQ(registry.numEntities(), 2);
    ASSERT_EQ(registry.numEntitiesAlive(), 2);
    ASSERT_EQ(registry.numEntitiesDead(), 0);

    registry.destroyEntity(entities[0]);
    registry.destroyEntity(entities[1]);
    ASSERT_EQ(registry.numEntities(), 2);
    ASSERT_EQ(registry.numEntitiesAlive(), 0);
    ASSERT_EQ(registry.numEntitiesDead(), 2);
    ASSERT_FALSE(registry.isEntityAlive(entities[0]));
    ASSERT_FALSE(registry.isEntityAlive(entities[1]));

    const std::array newEntities = { registry.createEntity(), registry.createEntity() };
    ASSERT_TRUE(registry.isEntityAlive(newEntities[0]));
    ASSERT_TRUE(registry.isEntityAlive(newEntities[1]));
    ASSERT_EQ(registry.numEntities(), 2);
    ASSERT_EQ(registry.numEntitiesAlive(), 2);
    ASSERT_EQ(registry.numEntitiesDead(), 0);
}

TEST(RegistryComponentHolderGrowingFixture, CreateEntity_CheckIfAlive) {
    Registry registry;
    const auto entity = registry.createEntity();
    ASSERT_TRUE(registry.isEntityAlive(entity));
    ASSERT_EQ(entity, Entity{ 0 });
    const auto entity2 = registry.createEntity();
    ASSERT_TRUE(registry.isEntityAlive(entity2));
}

TEST(RegistryComponentHolderGrowingFixture, CreateEntity_AttachComponent_CheckExistence) {
    Registry registry;
    const auto entity = registry.createEntity();
    ASSERT_FALSE(registry.hasComponent<Position>(entity));
    registry.attachComponent<Position>(entity, { 1.0f, 2.0f });
    ASSERT_TRUE(registry.hasComponent<Position>(entity));
    const auto entity2 = registry.createEntity();
    ASSERT_FALSE(registry.hasComponent<Velocity>(entity));
    ASSERT_FALSE(registry.hasComponent<Velocity>(entity2));
    registry.attachComponent<Velocity>(entity2, { 4.0f, 5.0f });
    ASSERT_FALSE(registry.hasComponent<Velocity>(entity));
    ASSERT_TRUE(registry.hasComponent<Velocity>(entity2));

    ASSERT_TRUE(registry.hasComponent<Position>(entity));
    ASSERT_FALSE(registry.hasComponent<Position>(entity2));
}

TEST(RegistryComponentHolderGrowingFixture, CreateEntity_AttachComponent_GetComponent) {
    Registry registry;
    const auto entity = registry.createEntity();
    registry.attachComponent(entity, Position{ 1.0f, 2.0f });
    auto&& position = registry.component<Position>(entity);
    ASSERT_TRUE(position);
    ASSERT_EQ(1.0f, position->x);
    ASSERT_EQ(2.0f, position->y);
}

TEST(RegistryComponentHolderGrowingFixture, CreateEntities_AttachComponents_CountComponents) {
    Registry registry;
    using ranges::views::ints, ranges::views::enumerate;
    std::vector<Entity> entities;
    for ([[maybe_unused]] auto i : ints(0, 100)) {
        entities.push_back(registry.createEntity());
    }
    for (auto entity : entities) {
        ASSERT_TRUE(registry.isEntityAlive(entity));
    }
    for (auto&& [i, entity] : entities | enumerate) {
        if (i % 2 == 0) {
            registry.attachComponent(entity, Position{ 2.0f, 3.0f });
        }
        if (i < 50) {
            registry.attachComponent(entity, Velocity{ 3.0f, 4.0f });
        }
    }
    for (auto&& [i, entity] : entities | enumerate) {
        ASSERT_EQ(registry.hasComponent<Position>(entity), i % 2 == 0);
        ASSERT_EQ(registry.hasComponent<Velocity>(entity), i < 50);
    }
    ASSERT_EQ(25, ranges::distance(registry.components<Position, Velocity>()));
    for (auto&& [i, tuple] : registry.components<Position, Velocity>() | enumerate) {
        const auto entity = std::get<0>(tuple);
        ASSERT_EQ(entities[i * 2], entity);
    }
    for (auto&& [i, entity] : entities | enumerate) {
        ASSERT_EQ(registry.hasComponent<Position>(entity), i % 2 == 0);
        ASSERT_EQ(registry.hasComponent<Velocity>(entity), i < 50);
    }
}

TEST(RegistryComponentHolderGrowingFixture, CreateEntities_AttachComponents_ModifyComponents) {
    Registry registry;
    using ranges::views::ints, ranges::views::enumerate;
    std::vector<Entity> entities;
    for ([[maybe_unused]] auto i : ints(0, 100)) {
        entities.push_back(registry.createEntity());
    }
    for (auto entity : entities) {
        registry.attachComponent(entity, Position{ 0.0f, 0.0f });
    }
    for (auto&& [entity, position] : registry.componentsMutable<Position>()) {
        position.x = 1.0f;
    }
    for (auto&& [i, tuple] : registry.components<Position>() | enumerate) {
        ASSERT_EQ(entities[i], std::get<0>(tuple));
        const auto& position = std::get<1>(tuple);
        ASSERT_EQ(position.x, 1.0f);
    }
}

TEST(RegistryComponentHolderGrowingFixture, CreateEntities_AttachComponents_IterateAndCheckOtherComponents) {
    Registry registry;
    using ranges::views::ints, ranges::views::enumerate;
    std::vector<Entity> entities;
    for ([[maybe_unused]] auto i : ints(0, 10)) {
        entities.push_back(registry.createEntity());
    }
    for (auto&& [i, entity] : entities | enumerate) {
        registry.attachComponent(entity, Position{ 1.0f, 2.0f });
        if (i % 2 == 0) {
            registry.attachComponent(entity, Velocity{ 4.0f, 5.0f });
        }
    }
    for (auto&& [i, tuple] : registry.components<Position>() | enumerate) {
        auto&& entity = std::get<0>(tuple);
        auto&& position = std::get<1>(tuple);
        ASSERT_EQ(entity, entities[i]);
        ASSERT_EQ(position.x, 1.0f);
        ASSERT_EQ(position.y, 2.0f);
        auto&& velocity = registry.component<Velocity>(entity);
        ASSERT_EQ(static_cast<bool>(velocity), i % 2 == 0);
        if (i % 2 == 0) {
            ASSERT_EQ(velocity->x, 4.0f);
            ASSERT_EQ(velocity->y, 5.0f);
        }
        auto&& velocityMutable = registry.componentMutable<Velocity>(entity);
        if (i % 2 == 0) {
            velocityMutable->x = 10.0f;
        }
    }
    ASSERT_EQ(ranges::distance(registry.components<Velocity>()), 5);
    for (auto&& [entity, velocity] : registry.components<Velocity>()) {
        ASSERT_EQ(velocity.x, 10.0f);
        ASSERT_EQ(velocity.y, 5.0f);
    }
}

TEST(RegistryEntityCreationAndDestructionFixture, CreateEntity_CheckIfAlive) {
    Registry registry{ 100 };
    const auto entity = registry.createEntity();
    ASSERT_TRUE(registry.isEntityAlive(entity));
    ASSERT_EQ(entity, Entity{ 0 });
}

struct Health {
    int value;
};

struct Mana {
    int value;
};

TEST(RegistryTests, TypeErasedIterating) {
    Registry registry;
    std::vector<Entity> entities;
    for (auto i : ranges::views::ints(0, 6)) {
        if (i % 2 == 0) {
            entities.push_back(registry.createEntity(Health{ 100 + i }, Mana{ 500 + i }));
        } else {
            entities.push_back(registry.createEntity(Health{ 100 + i }));
        }
    }
    std::array<std::size_t, 2> typeIdentifiers{ registry.typeIdentifier<Health>(), registry.typeIdentifier<Mana>() };
    {
        auto&& [begin, end] = registry.componentsTypeErased(std::begin(typeIdentifiers), std::end(typeIdentifiers));
        auto count{ 0 };
        for (auto it = begin; it != end; ++it) {
            const auto index = static_cast<std::size_t>(2 * count);// only even indices
            ASSERT_EQ((*it).index(), entities[index]);
            ASSERT_EQ(static_cast<const Health*>((*it).get(0))->value, 100 + index);
            ASSERT_EQ(static_cast<const Mana*>((*it).get(1))->value, 500 + index);
            ++count;
        }
        ASSERT_EQ(3, count);
    }
    {
        auto&& [begin, end] =
                registry.componentsTypeErasedMutable(std::begin(typeIdentifiers), std::end(typeIdentifiers));
        for (auto it = begin; it != end; ++it) {
            static_cast<Health*>((*it).get(0))->value += 10;
        }
    }
    {
        auto&& [begin, end] = registry.componentsTypeErased(std::begin(typeIdentifiers), std::end(typeIdentifiers));
        auto count{ 0 };
        for (auto it = begin; it != end; ++it) {
            const auto index = static_cast<std::size_t>(2 * count);// only even indices
            ASSERT_EQ((*it).index(), entities[index]);
            ASSERT_EQ(static_cast<const Health*>((*it).get(0))->value, 100 + index + 10 /* ! */);
            ASSERT_EQ(static_cast<const Mana*>((*it).get(1))->value, 500 + index);
            ++count;
        }
        ASSERT_EQ(3, count);
    }
    {
        std::array<std::size_t, 1> otherTypeIdentifiers{ registry.typeIdentifier<Health>() };
        auto&& [begin, end] =
                registry.componentsTypeErased(std::begin(otherTypeIdentifiers), std::end(otherTypeIdentifiers));
        auto count{ 0 };
        for (auto it = begin; it != end; ++it) {
            ASSERT_EQ((*it).index(), entities[count]);
            ASSERT_EQ(static_cast<const Health*>((*it).get(0))->value, 100 + count + (count % 2 == 0 ? 10 : 0));
            ++count;
        }
        ASSERT_EQ(6, count);
    }
}