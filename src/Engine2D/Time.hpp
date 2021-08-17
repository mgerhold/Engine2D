//
// Created by coder2k on 16.08.2021.
//

#pragma once

#include <chrono>

struct Time {
    std::chrono::duration<double, std::ratio<1, 1>> delta{ std::chrono::milliseconds{ 1000 / 60 } };
};