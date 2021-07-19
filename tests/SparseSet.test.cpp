//
// Created by coder2k on 19.07.2021.
//

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
    SparseSet<Position, Entity> positions{ 100 };
}

namespace {
    class SparseSetInsertionTest : public ::testing::Test {
    protected:
        SparseSet<Position, Entity> positions{ 100 };
    };

    TEST_F(SparseSetInsertionTest, InsertingValue_ReadValue) {
        constexpr Entity entity = 0;
        auto valueToBeInserted = Position{ .x = 1.0f, .y = 2.0f };
        positions.addComponent(entity, valueToBeInserted);
        const auto readValue = positions.getComponent(entity);
        ASSERT_EQ(valueToBeInserted, readValue);
    }

}// namespace