//
// Created by coder2k on 17.08.2021.
//

#include <GUID.hpp>
#include <gtest/gtest.h>
#include <range/v3/all.hpp>
#include <vector>
#include <algorithm>

TEST(GUIDTests, NoDuplicates) {
    using ranges::views::ints;
    constexpr std::size_t numElements{ 1000 };// well...not that many tbh
    std::vector<GUID> guids;
    guids.reserve(numElements);
    for (auto _ : ints(std::size_t{ 0 }, numElements)) {
        guids.push_back(GUID::create());
    }
    auto firstNotUnique = std::unique(guids.begin(), guids.end());
    ASSERT_EQ(firstNotUnique, guids.end());
}

TEST(GUIDTests, ConvertToStringAndBack) {
    const auto guid = GUID::create();
    const auto string = guid.string();
    const auto converted = GUID::fromString(string);
    ASSERT_EQ(converted, guid);
}