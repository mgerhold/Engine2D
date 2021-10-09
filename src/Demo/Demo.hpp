//
// Created by coder2k on 09.10.2021.
//

#pragma once

#include <Application.hpp>

class Demo : public c2k::Application {
    using c2k::Application::Application;

    void setup() noexcept override;
    void update() noexcept override;
};
