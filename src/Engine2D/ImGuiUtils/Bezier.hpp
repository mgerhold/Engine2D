// https://github.com/ocornut/imgui/issues/786

#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#pragma warning(push)
#pragma warning(disable : 5054)
#include <imgui_internal.h>
#pragma warning(pop)
#include <glm/glm.hpp>

namespace ImGui {

    float BezierValue(float dt01, glm::vec2 p0, glm::vec2 p1, float minVal = 0.0f, float maxVal = 1.0f);

    int Bezier(const char* label,
               glm::vec2* p0,
               glm::vec2* p1,
               float* minVal,
               float* maxVal,
               int* selectedPreset,
               float speed);

}// namespace ImGui