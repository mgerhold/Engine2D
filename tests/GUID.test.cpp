//
// Created by coder2k on 17.08.2021.
//

#include <GUID.hpp>
#include <gtest/gtest.h>
#include <boost/uuid/uuid.hpp>
#include <range/v3/all.hpp>
#include <algorithm>
#include <utility>
#include <vector>
#include <cstddef>

TEST(GUIDTests, NoDuplicates) {
    using ranges::views::ints;
    constexpr std::size_t numElements{ 1000 };// well...not that many tbh
    std::vector<c2k::GUID> guids;
    guids.reserve(numElements);
    for ([[maybe_unused]] auto _ : ints(std::size_t{ 0 }, numElements)) {
        guids.push_back(c2k::GUID::create());
    }
    auto firstNotUnique = std::unique(guids.begin(), guids.end());
    ASSERT_EQ(firstNotUnique, guids.end());
}

TEST(GUIDTests, ConvertToStringAndBack) {
    const auto guid = c2k::GUID::create();
    const auto string = guid.string();
    const auto converted = c2k::GUID::fromString(string);
    ASSERT_EQ(converted, guid);
}