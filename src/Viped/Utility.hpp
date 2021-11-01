//
// Created by coder2k on 01.11.2021.
//

#pragma once

#include <imgui.h>

void dragDouble(const char* label,
                double* v,
                double vSpeed = 1.0,
                double vMin = 0.0,
                double vMax = 0.0,
                const char* format = "%.3f",
                ImGuiSliderFlags flags = 0) noexcept;
