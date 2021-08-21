//
// Created by coder2k on 21.08.2021.
//

#include "Random.hpp"

namespace c2k {

    Random::Random() noexcept : mRandomEngine{ mRandomDevice() } { }

}// namespace c2k