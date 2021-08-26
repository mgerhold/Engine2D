//
// Created by coder2k on 19.07.2021.
//

#include <Entity.hpp>
#include <SparseSet.hpp>
#include <gtest/gtest.h>

using namespace c2k;

struct Position {
    float x{ 0.0f }, y{ 0.0f };

    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

using Entity = uint32_t;

TEST(SparseSetTests, CreateInstance) {
    SparseSet<Entity, invalidEntity<Entity>> positions{ Tag<Position>{}, 100 };
}

namespace {
    class SparseSetInsertionTest : public ::testing::Test {
    protected:
        SparseSet<Entity, invalidEntity<Entity>> positions{ Tag<Position>{}, 100 };
    };

    TEST_F(SparseSetInsertionTest, InsertingValue_ReadValue) {
        constexpr Entity entity = 0;
        const auto valueToBeInserted = Position{ .x = 1.0f, .y = 2.0f };
        positions.add(entity, valueToBeInserted);
        const auto readValue = positions.get<Position>(entity);
        ASSERT_EQ(valueToBeInserted, readValue);
    }

}// namespace

struct Health {
    int value;
};

TEST(SparseSetTests, TypeErasedIteration) {
    SparseSet<Entity, invalidEntity<Entity>> healthValues{ Tag<Health>{}, 100 };
    healthValues.add(20, Health{ 100 });
    healthValues.add(10, Health{ 101 });
    healthValues.add(40, Health{ 102 });
    int counter{ 0 };
    for (auto& ptr : healthValues.typeErasedElements()) {
        ASSERT_EQ(static_cast<const Health*>(ptr)->value, 100 + counter);
        ++counter;
    }
    ASSERT_EQ(static_cast<const Health*>(healthValues.getTypeErased(10))->value, 101);
}