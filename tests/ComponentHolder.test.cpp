//
// Created by coder2k on 26.08.2021.
//

#include <ComponentHolder.hpp>
#include <TypeIdentifier.hpp>
#include <ComponentHolderPairIterator.hpp>
#include <Entity.hpp>
#include <gtest/gtest.h>
#include <range/v3/all.hpp>
#include <gsl/gsl>
#include <array>
#include <iterator>
#include <new>
#include <utility>
#include <vector>
#include <cstddef>

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
                ASSERT_EQ(entity, Entity{ 0 });
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
                ASSERT_EQ(entity, Entity{ 0 });
                ASSERT_EQ(static_cast<const Health*>(healthPtr)->value, 100);
                ASSERT_EQ(static_cast<const Mana*>(manaPtr)->value, 102);
                break;
            case 1:
                ASSERT_EQ(entity, Entity{ 1 });
                ASSERT_EQ(static_cast<const Health*>(healthPtr)->value, 101);
                ASSERT_EQ(static_cast<const Mana*>(manaPtr)->value, 103);
                break;
            default:
                ASSERT_TRUE(false);
                break;
        }
    }
}

TEST(ComponentHolderTests, TypeErasedIterating_Range) {
    ComponentHolder<Entity> componentHolder(100);
    for (auto i : ranges::views::ints(0, 5)) {
        componentHolder.attach(i, Health{ 100 + i });
        componentHolder.attach(i, Mana{ 500 + i });
    }
    componentHolder.attach(5, Mana{ 500 + 5 });
    componentHolder.attach(6, Mana{ 500 + 6 });
    componentHolder.attach(7, Mana{ 500 + 7 });
    const std::vector<std::size_t> typeIdentifiers{ componentHolder.typeIdentifier<Health>(),
                                                    componentHolder.typeIdentifier<Mana>() };
    {
        auto&& [begin, end] = componentHolder.getTypeErased(typeIdentifiers.cbegin(), typeIdentifiers.cend());
        int count{ 0 };
        for (auto it{ begin }; it != end; ++it) {
            ASSERT_EQ(count, (*it).index());
            ASSERT_EQ(100 + count, static_cast<const Health*>((*it).get(0))->value);
            ASSERT_EQ(500 + count, static_cast<const Mana*>((*it).get(1))->value);
            ++count;
        }
        ASSERT_EQ(count, 5);
    }
    componentHolder.attach(6, Health{ 1000 });
    {
        auto&& [begin, end] = componentHolder.getTypeErased(typeIdentifiers.cbegin(), typeIdentifiers.cend());
        int count{ 0 };
        for (auto it{ begin }; it != end; ++it) {
            if (count < 5) {
                ASSERT_EQ(count, (*it).index());
                ASSERT_EQ(100 + count, static_cast<const Health*>((*it).get(0))->value);
                ASSERT_EQ(500 + count, static_cast<const Mana*>((*it).get(1))->value);
            } else {
                ASSERT_EQ(6, (*it).index());
                ASSERT_EQ(1000, static_cast<const Health*>((*it).get(0))->value);
                ASSERT_EQ(506, static_cast<const Mana*>((*it).get(1))->value);
            }
            ++count;
        }
        ASSERT_EQ(count, 6);
    }
}

TEST(ComponentHolderTests, TypeErasedIterating_Range_Mutable) {
    ComponentHolder<Entity> componentHolder(100);
    for (auto i : ranges::views::ints(0, 5)) {
        componentHolder.attach(i, Health{ 100 + i });
    }
    std::array<std::size_t, 1> typeIdentifiers{ componentHolder.typeIdentifier<Health>() };
    {
        // assert on all pre-filled values
        auto&& [begin, end] = componentHolder.getTypeErased(std::begin(typeIdentifiers), std::end(typeIdentifiers));
        int count{ 0 };
        for (auto it{ begin }; it != end; ++it) {
            ASSERT_EQ(count, (*it).index());
            ASSERT_EQ(100 + count, static_cast<const Health*>((*it).get(0))->value);
            ++count;
        }
    }
    {
        // mutate values
        auto&& [begin, end] =
                componentHolder.getTypeErasedMutable(std::begin(typeIdentifiers), std::end(typeIdentifiers));
        for (auto it{ begin }; it != end; ++it) {
            static_cast<Health*>((*it).get(0))->value += 10;
        }
    }
    {
        // check that mutations were successful
        auto&& [begin, end] = componentHolder.getTypeErased(std::begin(typeIdentifiers), std::end(typeIdentifiers));
        int count{ 0 };
        for (auto it{ begin }; it != end; ++it) {
            ASSERT_EQ(count, (*it).index());
            ASSERT_EQ(100 + count + 10 /* ! */, static_cast<const Health*>((*it).get(0))->value);
            ++count;
        }
    }
}