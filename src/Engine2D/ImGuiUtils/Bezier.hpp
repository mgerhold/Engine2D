// https://github.com/ocornut/imgui/issues/786

#pragma once

#include "ParticleSystem.hpp"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <IncludeImGuiInternal.hpp>
#include <glm/glm.hpp>

namespace ImGui {

    float BezierValue(float dt01, const c2k::ParticleSystemImpl::BezierCurve& curve);

    int Bezier(const char* label,
               c2k::ParticleSystemImpl::BezierCurve* curve,
               int* activeHandle,
               float speed,
               float min = 0.0f,
               float max = 0.0f);

}// namespace ImGui