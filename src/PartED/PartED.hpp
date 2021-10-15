//
// Created by coder2k on 09.10.2021.
//

#pragma once

#include <Application.hpp>
#include "GUID.hpp"

class PartED : public c2k::Application {
private:
    using c2k::Application::Application;

    void setup() noexcept override;
    void update() noexcept override;
    void renderImGui() noexcept override;

private:
    c2k::GUID mTextureGUID;
    c2k::GUID mShaderProgramGUID;
    c2k::Entity mParticleEmitterEntity{ c2k::invalidEntity };
};
