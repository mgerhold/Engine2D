//
// Created by coder2k on 01.11.2021.
//

#include "Utility.hpp"
#include <gsl/gsl>

void dragDouble(const char* label,
                double* v,
                double vSpeed,
                double vMin,
                double vMax,
                const char* format,
                ImGuiSliderFlags flags) noexcept {
    float value = gsl::narrow_cast<float>(*v);
    ImGui::DragFloat(label, &value, gsl::narrow_cast<float>(vSpeed), gsl::narrow_cast<float>(vMin),
                     gsl::narrow_cast<float>(vMax), format, flags);
    *v = static_cast<double>(value);
}