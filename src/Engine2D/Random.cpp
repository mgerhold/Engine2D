//
// Created by coder2k on 21.08.2021.
//

#include "Random.hpp"

Random::Random() noexcept : mRandomEngine{ mRandomDevice() } { }
