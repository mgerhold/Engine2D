//
// Created by coder2k on 19.07.2021.
//

#include <Entity.hpp>
#include <SparseSet.hpp>
#include <gtest/gtest.h>
#include <cstdint>

struct Position {
    float x, y;

    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

using Entity = uint32_t;

TEST(SparseSetTests, CreateInstance) {
    SparseSet<Position, Entity, invalidEntity<Entity>> positions{ 100 };
}

namespace {
    class SparseSetInsertionTest : public ::testing::Test {
    protected:
        SparseSet<Position, Entity, invalidEntity<Entity>> positions{ 100 };
    };

    TEST_F(SparseSetInsertionTest, InsertingValue_ReadValue) {
        constexpr Entity entity = 0;
        const auto valueToBeInserted = Position{ .x = 1.0f, .y = 2.0f };
        positions.add(entity, valueToBeInserted);
        const auto readValue = positions.get(entity);
        ASSERT_EQ(valueToBeInserted, readValue);
    }

}// namespace