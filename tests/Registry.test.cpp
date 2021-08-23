//
// Created by coder2k on 31.07.2021.
//

#include <Registry.hpp>
#include <TypeIdentifier.hpp>
#include <gtest/gtest.h>
#include <pch.hpp>

using namespace c2k;

struct Position {
    float x, y;
};

struct Velocity {
    float x, y;
};

namespace {
    class RegistryComponentHolderGrowingFixture : public ::testing::Test {
    protected:
        Registry registry;
    };

    TEST_F(RegistryComponentHolderGrowingFixture, CreateEntity_CheckIfAlive) {
        const auto entity = registry.createEntity();
        ASSERT_TRUE(registry.isEntityAlive(entity));
        ASSERT_EQ(entity, Entity{ 0 });
        const auto entity2 = registry.createEntity();
        ASSERT_TRUE(registry.isEntityAlive(entity2));
    }

    TEST_F(RegistryComponentHolderGrowingFixture, CreateEntity_AttachComponent_CheckExistence) {
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

    TEST_F(RegistryComponentHolderGrowingFixture, CreateEntity_AttachComponent_GetComponent) {
        const auto entity = registry.createEntity();
        registry.attachComponent(entity, Position{ 1.0f, 2.0f });
        auto&& position = registry.component<Position>(entity);
        ASSERT_TRUE(position);
        ASSERT_EQ(1.0f, position->x);
        ASSERT_EQ(2.0f, position->y);
    }

    TEST_F(RegistryComponentHolderGrowingFixture, CreateEntities_AttachComponents_CountComponents) {
        using ranges::views::ints, ranges::views::enumerate;
        std::vector<typename decltype(registry)::EntityType> entities;
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

    TEST_F(RegistryComponentHolderGrowingFixture, CreateEntities_AttachComponents_ModifyComponents) {
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

    TEST_F(RegistryComponentHolderGrowingFixture, CreateEntities_AttachComponents_IterateAndCheckOtherComponents) {
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

    TEST_F(RegistryComponentHolderGrowingFixture, TriggerTypeIdentifierCreation_UseInDifferentOrder) {
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

    class RegistryEntityCreationAndDestructionFixture : public ::testing::Test {
    protected:
        Registry registry{ 100 };
    };

    TEST_F(RegistryEntityCreationAndDestructionFixture, CreateEntity_CheckIfAlive) {
        const auto entity = registry.createEntity();
        ASSERT_TRUE(registry.isEntityAlive(entity));
        ASSERT_EQ(entity, Entity{ 0 });
    }

    TEST_F(RegistryEntityCreationAndDestructionFixture, RecycleEntity_CheckAliveStatus) {
        ASSERT_EQ(registry.numEntities(), 0);
        const Entity entities[] = { registry.createEntity(), registry.createEntity() };
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

        const Entity newEntities[] = { registry.createEntity(), registry.createEntity() };
        ASSERT_TRUE(registry.isEntityAlive(newEntities[0]));
        ASSERT_TRUE(registry.isEntityAlive(newEntities[1]));
        ASSERT_EQ(registry.numEntities(), 2);
        ASSERT_EQ(registry.numEntitiesAlive(), 2);
        ASSERT_EQ(registry.numEntitiesDead(), 0);
    }
}// namespace