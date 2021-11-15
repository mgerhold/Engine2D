//
// Created by coder2k on 01.11.2021.
//

#pragma once

#include <imgui.h>
#include <type_traits>

void dragDouble(const char* label,
                double* v,
                double vSpeed = 1.0,
                double vMin = 0.0,
                double vMax = 0.0,
                const char* format = "%.3f",
                ImGuiSliderFlags flags = 0) noexcept;

template<typename T>
void drag(const char* label, T* v, T vSpeed = T{ 1 }, T vMin = T{ 0 }, T vMax ={ 0 }, const char* format = "%.3f",
          ImGuiSliderFlags flags = 0) noexcept {
    if constexpr (std::is_same_v<T, float>) {
        ImGui::DragFloat(label, v, vSpeed, vMin, vMax, format, flags);
    } else if constexpr (std::is_same_v<T, double>) {
        dragDouble(label, v, vSpeed, vMin, vMax, format, flags);
    } else {
        static_assert(false);
    }
}