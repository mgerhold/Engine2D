//
// Created by coder2k on 22.11.2021.
//

#include "ColorGradient.hpp"

//
//  imgui_color_gradient.cpp
//  imgui extension
//
//  Created by David Gallardo on 11/06/16.

#pragma warning(push)
#pragma warning(disable : 5054)
#include "imgui_internal.h"
#pragma warning(pop)
#include <gsl/gsl>
#include <algorithm>
#include <utility>
#include <cstdint>

static const float GRADIENT_BAR_WIDGET_HEIGHT = 25;
static const float GRADIENT_BAR_EDITOR_HEIGHT = 40;
static const float GRADIENT_MARK_DELETE_DIFFY = 40;

ImGradient::ImGradient() {
    addMark(0.0f, ImColor(0.0f, 0.0f, 0.0f));
    addMark(1.0f, ImColor(1.0f, 1.0f, 1.0f));
}

void ImGradient::addMark(float position, ImColor color) {
    mMarks.emplace_back(ImGradientMark{ std::move(color), ImClamp(position, 0.0f, 1.0f) });
    refreshCache();
}

void ImGradient::removeMark(Marks::iterator iterator) {
    mMarks.erase(iterator);
    refreshCache();
}

ImColor ImGradient::getColorAt(float position) const {
    position = ImClamp(position, 0.0f, 1.0f);
    const auto cachePos = gsl::narrow_cast<std::size_t>(position * 255);
    return mCachedValues[cachePos];
}

ImColor ImGradient::computeColorAt(float position) const {
    if (mMarks.empty()) {
        return ImColor{ 0.0f, 0.0f, 0.0f, 1.0f };
    }
    position = ImClamp(position, 0.0f, 1.0f);

    const auto [lower, upper] = findBounds(position);

    if (upper == lower) {
        return upper->color;
    } else {
        const float distance = upper->position - lower->position;
        const float interpolationParameter = (position - lower->position) / distance;
        return lerpColor(lower->color, upper->color, interpolationParameter);
    }
}

void ImGradient::refreshCache() {
    std::sort(mMarks.begin(), mMarks.end(),
              [](const auto& lhs, const auto& rhs) { return lhs.position < rhs.position; });

    for (int i = 0; i < 256; ++i) {
        mCachedValues[i] = computeColorAt(static_cast<float>(i) / 255.0f);
    }
}

ImGradient::MarkIteratorPair ImGradient::findBounds(float position) const {
    auto upper = std::upper_bound(mMarks.begin(), mMarks.end(), position,
                                  [](const float value, const auto& mark) { return value < mark.position; });
    if (upper == mMarks.end()) {
        --upper;
    }
    auto lower = upper;
    if (lower != mMarks.begin() && upper != mMarks.end() - 1) {
        --lower;
    }
    return MarkIteratorPair{ lower, upper };
}

ImColor ImGradient::lerpColor(const ImColor& lhs, const ImColor& rhs, float interpolationParameter) {
    return ImColor{ lhs.Value.x * (1.0f - interpolationParameter) + rhs.Value.x * interpolationParameter,
                    lhs.Value.y * (1.0f - interpolationParameter) + rhs.Value.y * interpolationParameter,
                    lhs.Value.z * (1.0f - interpolationParameter) + rhs.Value.z * interpolationParameter,
                    lhs.Value.w * (1.0f - interpolationParameter) + rhs.Value.w * interpolationParameter };
}

namespace ImGui {
    static void DrawGradientBar(ImGradient* gradient, struct ImVec2 const& bar_pos, float maxWidth, float height) {
        const float barBottom = bar_pos.y + height;
        const ImGradientMark* prevMark = nullptr;
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(ImVec2(bar_pos.x - 2, bar_pos.y - 2), ImVec2(bar_pos.x + maxWidth + 2, barBottom + 2),
                                 IM_COL32(100, 100, 100, 255));
        if (gradient->getMarks().size() == 0) {
            draw_list->AddRectFilled(ImVec2(bar_pos.x, bar_pos.y), ImVec2(bar_pos.x + maxWidth, barBottom),
                                     IM_COL32(255, 255, 255, 255));
        }

        float prevX = bar_pos.x;
        ImColor colorBU32;
        for (const auto& mark : gradient->getMarks()) {
            float from = prevX;
            float to = prevX = bar_pos.x + mark.position * maxWidth;

            const auto colorA = (prevMark == nullptr ? mark.color : prevMark->color);
            const auto colorB = mark.color;

            const auto colorAU32 = ImGui::ColorConvertFloat4ToU32(colorA);
            colorBU32 = ImGui::ColorConvertFloat4ToU32(colorB);

            if (mark.position > 0.0) {
                draw_list->AddRectFilledMultiColor(ImVec2(from, bar_pos.y), ImVec2(to, barBottom), colorAU32, colorBU32,
                                                   colorBU32, colorAU32);
            }

            prevMark = &mark;
        }

        if (prevMark && prevMark->position < 1.0f) {
            draw_list->AddRectFilledMultiColor(ImVec2(prevX, bar_pos.y), ImVec2(bar_pos.x + maxWidth, barBottom),
                                               colorBU32, colorBU32, colorBU32, colorBU32);
        }
        ImGui::SetCursorScreenPos(ImVec2(bar_pos.x, bar_pos.y + height + 10.0f));
    }

    static void DrawGradientMarks(ImGradient* gradient,
                                  ImGradientMark*& draggingMark,
                                  ImGradientMark*& selectedMark,
                                  struct ImVec2 const& bar_pos,
                                  float maxWidth,
                                  float height) {
        float barBottom = bar_pos.y + height;
        ImGradientMark* prevMark = nullptr;
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImU32 colorAU32 = 0;
        ImU32 colorBU32 = 0;

        for (auto& mark : gradient->getMarks()) {
            if (!selectedMark) {
                selectedMark = &mark;
            }

            float to = bar_pos.x + mark.position * maxWidth;

            const auto colorA = (prevMark == nullptr ? mark.color : prevMark->color);
            const auto colorB = mark.color;

            colorAU32 = ImGui::ColorConvertFloat4ToU32(colorA);
            colorBU32 = ImGui::ColorConvertFloat4ToU32(colorB);

            draw_list->AddTriangleFilled(ImVec2(to, bar_pos.y + (height - 6)), ImVec2(to - 6, barBottom),
                                         ImVec2(to + 6, barBottom), IM_COL32(100, 100, 100, 255));

            draw_list->AddRectFilled(ImVec2(to - 6, barBottom), ImVec2(to + 6, bar_pos.y + (height + 12)),
                                     IM_COL32(100, 100, 100, 255), 1.0f, 0);

            draw_list->AddRectFilled(ImVec2(to - 5, bar_pos.y + (height + 1)),
                                     ImVec2(to + 5, bar_pos.y + (height + 11)), IM_COL32(0, 0, 0, 255), 1.0f, 0);

            if (selectedMark == &mark) {
                draw_list->AddTriangleFilled(ImVec2(to, bar_pos.y + (height - 3)), ImVec2(to - 4, barBottom + 1),
                                             ImVec2(to + 4, barBottom + 1), IM_COL32(0, 255, 0, 255));

                draw_list->AddRect(ImVec2(to - 5, bar_pos.y + (height + 1)), ImVec2(to + 5, bar_pos.y + (height + 11)),
                                   IM_COL32(0, 255, 0, 255), 1.0f, 0);
            }

            draw_list->AddRectFilledMultiColor(ImVec2(to - 3, bar_pos.y + (height + 3)),
                                               ImVec2(to + 3, bar_pos.y + (height + 9)), colorBU32, colorBU32,
                                               colorBU32, colorBU32);

            ImGui::SetCursorScreenPos(ImVec2(to - 6, barBottom));
            ImGui::InvisibleButton("mark", ImVec2(12, 12));

            if (ImGui::IsItemHovered()) {
                if (ImGui::IsMouseClicked(0)) {
                    selectedMark = &mark;
                    draggingMark = &mark;
                }
            }

            prevMark = &mark;
        }

        ImGui::SetCursorScreenPos(ImVec2(bar_pos.x, bar_pos.y + height + 20.0f));
    }

    bool GradientButton(ImGradient* gradient) {
        if (!gradient)
            return false;

        ImVec2 widget_pos = ImGui::GetCursorScreenPos();

        float maxWidth = ImMax(250.0f, ImGui::GetContentRegionAvailWidth() - 100.0f);
        bool clicked = ImGui::InvisibleButton("gradient_bar", ImVec2(maxWidth, GRADIENT_BAR_WIDGET_HEIGHT));

        DrawGradientBar(gradient, widget_pos, maxWidth, GRADIENT_BAR_WIDGET_HEIGHT);

        return clicked;
    }

    bool GradientEditor(ImGradient* gradient, ImGradientMark*& draggingMark, ImGradientMark*& selectedMark) {
        if (!gradient)
            return false;

        bool modified = false;

        ImVec2 bar_pos = ImGui::GetCursorScreenPos();
        bar_pos.x += 10;
        float maxWidth = ImGui::GetContentRegionAvailWidth() - 20;
        float barBottom = bar_pos.y + GRADIENT_BAR_EDITOR_HEIGHT;

        ImGui::InvisibleButton("gradient_editor_bar", ImVec2(maxWidth, GRADIENT_BAR_EDITOR_HEIGHT));

        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
            float pos = (ImGui::GetIO().MousePos.x - bar_pos.x) / maxWidth;
            const auto newMarkColor = gradient->getColorAt(pos);
            gradient->addMark(pos, newMarkColor);
        }

        DrawGradientBar(gradient, bar_pos, maxWidth, GRADIENT_BAR_EDITOR_HEIGHT);
        DrawGradientMarks(gradient, draggingMark, selectedMark, bar_pos, maxWidth, GRADIENT_BAR_EDITOR_HEIGHT);

        if (!ImGui::IsMouseDown(0) && draggingMark) {
            draggingMark = nullptr;
        }

        if (ImGui::IsMouseDragging(0) && draggingMark) {
            float increment = ImGui::GetIO().MouseDelta.x / maxWidth;
            bool insideZone =
                    (ImGui::GetIO().MousePos.x > bar_pos.x) && (ImGui::GetIO().MousePos.x < bar_pos.x + maxWidth);

            if (increment != 0.0f && insideZone) {
                draggingMark->position += increment;
                draggingMark->position = ImClamp(draggingMark->position, 0.0f, 1.0f);
                gradient->refreshCache();
                modified = true;
            }

            float diffY = ImGui::GetIO().MousePos.y - barBottom;

            if (diffY >= GRADIENT_MARK_DELETE_DIFFY) {
                const auto findIterator =
                        std::find_if(gradient->getMarks().begin(), gradient->getMarks().end(),
                                     [&draggingMark](const auto& mark) { return &mark == draggingMark; });
                gradient->removeMark(findIterator);
                draggingMark = nullptr;
                selectedMark = nullptr;
                modified = true;
            }
        }

        if (!selectedMark && !gradient->getMarks().empty()) {
            selectedMark = &gradient->getMarks().front();
        }

        if (selectedMark) {
            std::array<float, 4> color{ selectedMark->color.Value.x, selectedMark->color.Value.y,
                                        selectedMark->color.Value.z, selectedMark->color.Value.w };
            constexpr ImGuiColorEditFlags flags = ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaBar;
            bool colorModified = ImGui::ColorPicker4("Preview", color.data(), flags);
            selectedMark->color = ImColor{ color[0], color[1], color[2], color[3] };

            if (selectedMark && colorModified) {
                modified = true;
                gradient->refreshCache();
            }
        }

        return modified;
    }
};// namespace ImGui