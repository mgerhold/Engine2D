//
// Created by coder2k on 22.08.2021.
//

#include <TypeErasedVector.hpp>
#include <gtest/gtest.h>
#include <pch.hpp>

using namespace c2k;

TEST(TypeErasedVectorTests, VectorOfIntegers) {
    auto numbers = TypeErasedVector::forType<int>();
    auto floats = TypeErasedVector::forType<float>();
    static_assert(std::is_same_v<decltype(numbers), decltype(floats)>);

    ASSERT_EQ(numbers.capacity(), 0);
    ASSERT_EQ(numbers.size(), 0);
    numbers.push_back(4);
    ASSERT_EQ(numbers.capacity(), 1);
    ASSERT_EQ(numbers.size(), 1);
    numbers.push_back(10);
    ASSERT_EQ(numbers.capacity(), 2);
    ASSERT_EQ(numbers.size(), 2);
    numbers.push_back(15);
    ASSERT_EQ(numbers.capacity(), 4);
    ASSERT_EQ(numbers.size(), 3);
    numbers.push_back(22);
    ASSERT_EQ(numbers.capacity(), 4);
    ASSERT_EQ(numbers.size(), 4);
    numbers.push_back(7);
    ASSERT_EQ(numbers.capacity(), 8);
    ASSERT_EQ(numbers.size(), 5);

    ASSERT_EQ(numbers.get<int>(0), 4);
    ASSERT_EQ(numbers.get<int>(1), 10);
    ASSERT_EQ(numbers.get<int>(2), 15);
    ASSERT_EQ(numbers.get<int>(3), 22);
    ASSERT_EQ(numbers.get<int>(4), 7);

    std::size_t i{ 0 };
    for (auto it = numbers.begin<int>(); it != numbers.end<int>(); ++it) {
        ASSERT_EQ(*it, numbers.get<int>(i));
        ++i;
    }
}

TEST(TypeErasedVectorTests, VectorOfStrings) {
    auto words = TypeErasedVector::forType<std::string>();

    words.push_back(std::string{ "Hello" });
    words.push_back(std::string{ "World" });
    words.push_back(std::string{ "!!!" });
    words.push_back(std::string{ "Lorem" });
    words.push_back(std::string{ "Ipsum" });

    ASSERT_EQ(words.capacity(), 8);
    ASSERT_EQ(words.size(), 5);

    ASSERT_EQ(words.get<std::string>(0), "Hello");
    ASSERT_EQ(words.get<std::string>(1), "World");
    ASSERT_EQ(words.get<std::string>(2), "!!!");
    ASSERT_EQ(words.get<std::string>(3), "Lorem");
    ASSERT_EQ(words.get<std::string>(4), "Ipsum");

    std::size_t i{ 0 };
    for (auto it = words.begin<std::string>(); it != words.end<std::string>(); ++it) {
        ASSERT_EQ(*it, words.get<std::string>(i));
        ++i;
    }
}

TEST(TypeErasedVectorTests, VectorOfUniquePointers) {
    auto pointers = TypeErasedVector::forType<std::unique_ptr<int>>();
    ASSERT_EQ(pointers.size(), 0);
    ASSERT_EQ(pointers.capacity(), 0);
    pointers.reserve(5);
    ASSERT_EQ(pointers.size(), 0);
    ASSERT_EQ(pointers.capacity(), 8);
    pointers.push_back(std::make_unique<int>(42));
    pointers.push_back(std::make_unique<int>(43));
    pointers.push_back(std::make_unique<int>(44));
    pointers.push_back(std::make_unique<int>(45));
    pointers.push_back(std::make_unique<int>(46));
    ASSERT_EQ(pointers.size(), 5);
    ASSERT_EQ(pointers.capacity(), 8);
    for (std::size_t i{ 0 }; i < pointers.size(); ++i) {
        ASSERT_EQ(42 + i, *pointers.get<std::unique_ptr<int>>(i));
    }
}

TEST(TypeErasedVectorTests, SwapElements) {
    auto pointers = TypeErasedVector::forType<std::unique_ptr<int>>();
    pointers.push_back(std::make_unique<int>(1));
    pointers.push_back(std::make_unique<int>(2));
    ASSERT_EQ(*pointers.get<std::unique_ptr<int>>(0), 1);
    ASSERT_EQ(*pointers.get<std::unique_ptr<int>>(1), 2);
    pointers.swapElements(0, 1);
    ASSERT_EQ(*pointers.get<std::unique_ptr<int>>(0), 2);
    ASSERT_EQ(*pointers.get<std::unique_ptr<int>>(1), 1);
}

struct S {
    S() : x{ 42 } { }
    int x;
};

TEST(TypeErasedVectorTests, Resize) {
    auto objects = TypeErasedVector::forType<S>();
    objects.resize(10);
    int count{ 0 };
    for (auto it = objects.begin<S>(); it != objects.end<S>(); ++it) {
        ASSERT_EQ((*it).x, 42);// TODO: implement -> operator
        ++count;
    }
    ASSERT_EQ(count, 10);
}