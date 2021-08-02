//
// Created by coder2k on 31.07.2021.
//

#include <Registry.hpp>
#include <gtest/gtest.h>

using Entity = uint32_t;

struct Position {
    float x, y;
};

struct Velocity {
    float x, y;
};

namespace {
    class RegistryComponentHolderGrowingFixture : public ::testing::Test {
    protected:
        Registry<Entity> registry{ 1 };
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
        registry.addComponent<Position>(entity, { 1.0f, 2.0f });
        ASSERT_TRUE(registry.hasComponent<Position>(entity));
        const auto entity2 = registry.createEntity();
        ASSERT_FALSE(registry.hasComponent<Velocity>(entity));
        ASSERT_FALSE(registry.hasComponent<Velocity>(entity2));
        registry.addComponent<Velocity>(entity2, { 4.0f, 5.0f });
        ASSERT_FALSE(registry.hasComponent<Velocity>(entity));
        ASSERT_TRUE(registry.hasComponent<Velocity>(entity2));

        ASSERT_TRUE(registry.hasComponent<Position>(entity));
        ASSERT_FALSE(registry.hasComponent<Position>(entity2));
    }

    class RegistryEntityCreationAndDestructionFixture : public ::testing::Test {
    protected:
        Registry<Entity> registry{ 100 };
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
}