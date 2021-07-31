//
// Created by coder2k on 31.07.2021.
//

#pragma once

#include <limits>

template<typename Entity>
static constexpr Entity invalidEntity = std::numeric_limits<Entity>::max();