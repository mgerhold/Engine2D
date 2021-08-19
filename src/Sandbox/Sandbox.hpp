//
// Created by coder2k on 15.05.2021.
//

#pragma once

#include "Application.hpp"

class Sandbox final : public Application<Sandbox> {
public:
    using Application::Application;

private:
    void setup() noexcept;
    void update() noexcept;

private:
    friend class Application;
};
