//
// Created by coder2k on 31.07.2021.
//

#include <Registry.hpp>
#include <gtest/gtest.h>

using Entity = uint32_t;

namespace {
    class RegistryEntityCreationAndDestructionFixture : public ::testing::Test {
    protected:
        Registry<Entity> registry{ 100 };
    };

    TEST_F(RegistryEntityCreationAndDestructionFixture, CreateEntity_CheckIfAlive) {
        const auto entity = registry.createEntity();
        ASSERT_TRUE(registry.isEntityAlive(entity));
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