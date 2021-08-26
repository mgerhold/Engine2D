//
// Created by coder2k on 26.08.2021.
//

#include <ComponentHolder.hpp>
#include <TypeIdentifier.hpp>
#include <Entity.hpp>
#include <gtest/gtest.h>

using namespace c2k;

struct Number {
    int n;
};

TEST(ComponentHolderTests, TypeErasedIterating) {
    using ranges::views::zip;
    ComponentHolder<Entity> componentHolder(100);
    std::array<Entity, 3> entities{ 42, 22, 25 };
    std::array<Number, 3> numbers{ Number{ 1 }, Number{ 2 }, Number{ 3 } };
    for (auto&& [entity, number] : zip(entities, numbers)) {
        componentHolder.attach(entity, number);
    }
    const auto typeIdentifier = componentHolder.typeIdentifier<Number>();
    int counter = 0;
    for (auto&& [entity, numberPtr] : componentHolder.getTypeErased(typeIdentifier)) {
        ASSERT_EQ(entity, entities[counter]);
        ASSERT_EQ(static_cast<const Number*>(numberPtr)->n, numbers[counter].n);
        ++counter;
    }
}

struct Health {
    int value;
};

struct Mana {
    int value;
};

TEST(ComponentHolderTests, TypeErasedIterating_MultipleComponents) {
    using ranges::views::enumerate;
    ComponentHolder<Entity> componentHolder(100);
    componentHolder.attach(0, Health{ 100 });
    componentHolder.attach(1, Health{ 101 });
    componentHolder.attach(0, Mana{ 102 });
    const auto healthID = componentHolder.typeIdentifier<Health>();
    const auto manaID = componentHolder.typeIdentifier<Mana>();
    // there are more Health instances than Mana instances
    for (auto&& [i, tuple] : componentHolder.getTypeErased(healthID, manaID) | enumerate) {
        auto&& [entity, healthPtr, manaPtr] = tuple;
        switch (i) {
            case 0:
                ASSERT_EQ(entity, 0);
                ASSERT_EQ(static_cast<const Health*>(healthPtr)->value, 100);
                ASSERT_EQ(static_cast<const Mana*>(manaPtr)->value, 102);
                break;
            default:
                ASSERT_TRUE(false);
                break;
        }
    }
    componentHolder.attach(1, Mana{ 103 });
    componentHolder.attach(2, Mana{ 104 });
    // now there are more Mana instances than Health instances
    for (auto&& [i, tuple] : componentHolder.getTypeErased(healthID, manaID) | enumerate) {
        auto&& [entity, healthPtr, manaPtr] = tuple;
        switch (i) {
            case 0:
                ASSERT_EQ(entity, 0);
                ASSERT_EQ(static_cast<const Health*>(healthPtr)->value, 100);
                ASSERT_EQ(static_cast<const Mana*>(manaPtr)->value, 102);
                break;
            case 1:
                ASSERT_EQ(entity, 1);
                ASSERT_EQ(static_cast<const Health*>(healthPtr)->value, 101);
                ASSERT_EQ(static_cast<const Mana*>(manaPtr)->value, 103);
                break;
            default:
                ASSERT_TRUE(false);
                break;
        }
    }
}