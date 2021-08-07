//
// Created by coder2k on 31.07.2021.
//

#pragma once

#include "strong_type/strong_type.hpp"
#include <limits>

template<typename Entity>
static constexpr Entity invalidEntity = std::numeric_limits<typename strong::underlying_type<Entity>::type>::max();

template<typename Entity>
using EntityBase = Entity;

using Entity = EntityBase<std::uint32_t>;