#include "Bezier.hpp"
#include "ParticleSystem.hpp"
#include <range/v3/all.hpp>
#include <gsl/gsl>
#include <algorithm>
#include <array>
#include <limits>

using namespace c2k::ParticleSystemImpl;

namespace ImGui {

    // visuals
    constexpr auto SMOOTHNESS = 64;        // curve smoothness: the higher number of segments, the smoother curve
    constexpr auto CURVE_WIDTH = 4;        // main curved line width
    constexpr auto LINE_WIDTH = 1;         // handlers: small lines width
    constexpr auto GRAB_RADIUS = 8;        // handlers: circle radius
    constexpr auto GRAB_BORDER = 2;        // handlers: circle border width
    constexpr auto AREA_CONSTRAINED = true;// should grabbers be constrained to grid area?
    constexpr auto AREA_WIDTH = 128;       // area width in pixels. 0 for adaptive size (will use max avail width)

    template<int steps>
    void bezier_table(const std::array<ImVec2, 4>& points, std::array<ImVec2, steps + 1>& results) {
        static float C[(steps + 1) * 4], *K = 0;
        if (!K) {
            K = C;
            for (unsigned step = 0; step <= steps; ++step) {
                float t = (float) step / (float) steps;
                C[step * 4 + 0] = (1 - t) * (1 - t) * (1 - t);// * P0
                C[step * 4 + 1] = 3 * (1 - t) * (1 - t) * t;  // * P1
                C[step * 4 + 2] = 3 * (1 - t) * t * t;        // * P2
                C[step * 4 + 3] = t * t * t;                  // * P3
            }
        }
        for (unsigned step = 0; step <= steps; ++step) {
            ImVec2 point = { K[step * 4 + 0] * points[0].x + K[step * 4 + 1] * points[1].x +
                                     K[step * 4 + 2] * points[2].x + K[step * 4 + 3] * points[3].x,
                             K[step * 4 + 0] * points[0].y + K[step * 4 + 1] * points[1].y +
                                     K[step * 4 + 2] * points[2].y + K[step * 4 + 3] * points[3].y };
            results[step] = point;
        }
    }

    float BezierValue(float dt01, const BezierCurve& curve) {
        constexpr auto STEPS = 256;
        std::array pivotPoints = { ImVec2{ 0.0f, curve.leftY }, ImVec2{ curve.p0.x, curve.p0.y },
                                   ImVec2{ curve.p1.x, curve.p1.y }, ImVec2{ 1.0f, curve.rightY } };
        std::array<ImVec2, STEPS + 1> results;
        bezier_table<STEPS>(pivotPoints, results);
        return results[(int) ((dt01 < 0   ? 0
                               : dt01 > 1 ? 1
                                          : dt01) *
                              STEPS)]
                               .y *
                       (curve.maxVal - curve.minVal) +
               curve.minVal;
    }

    int Bezier(const char* label, BezierCurve* curve, int* activeHandle, float speed, float min, float max) {
        // bezier widget
        const ImGuiStyle& Style = GetStyle();
        ImDrawList* DrawList = GetWindowDrawList();
        ImGuiWindow* Window = GetCurrentWindow();
        if (Window->SkipItems)
            return false;

        // header and spacing
        bool changed = false;

        ImGui::Text("(%0.2f, %0.2f), (%0.2f, %0.2f)", curve->p0.x, curve->p0.y, curve->p1.x, curve->p1.y);
        ImGui::DragFloat("min", &curve->minVal, speed, min, max);
        ImGui::DragFloat("max", &curve->maxVal, speed, min, max);

        bool hovered = IsItemActive() || IsItemHovered();
        Dummy(ImVec2(0, 3));

        // prepare canvas
        const float avail = GetContentRegionAvail().x;
        const float dim = AREA_WIDTH > 0 ? AREA_WIDTH : avail;
        ImVec2 Canvas(dim, dim);

        ImRect bb(Window->DC.CursorPos, Window->DC.CursorPos + Canvas);
        ItemSize(bb);
        if (!ItemAdd(bb, 0)) {
            *activeHandle = -1;
            return changed;
        }
        const auto getHandles = [&bb](const auto& points) {
            auto result = points;
            for (auto& handle : result) {
                handle.y = 1.0f - handle.y;
                handle = handle * (bb.Max - bb.Min) + bb.Min;
            }
            return result;
        };
        std::array points = { ImVec2{ 0.0f, curve->leftY }, ImVec2{ curve->p0.x, curve->p0.y },
                              ImVec2{ curve->p1.x, curve->p1.y }, ImVec2{ 1.0f, curve->rightY } };
        const auto handles = getHandles(points);

        const ImGuiID id = Window->GetID(label);
        hovered |= 0 != ItemHoverable(ImRect(bb.Min, bb.Min + ImVec2(avail, dim)), id);

        RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg, 1), true, Style.FrameRounding);

        // background grid
        for (int i = 0; i <= gsl::narrow_cast<int>(Canvas.x); i += gsl::narrow_cast<int>(Canvas.x / 4.0f)) {
            DrawList->AddLine(ImVec2(bb.Min.x + static_cast<float>(i), bb.Min.y),
                              ImVec2(bb.Min.x + static_cast<float>(i), bb.Max.y), GetColorU32(ImGuiCol_TextDisabled));
        }
        for (int i = 0; i <= gsl::narrow_cast<int>(Canvas.y); i += gsl::narrow_cast<int>(Canvas.y / 4.0f)) {
            DrawList->AddLine(ImVec2(bb.Min.x, bb.Min.y + static_cast<float>(i)),
                              ImVec2(bb.Max.x, bb.Min.y + static_cast<float>(i)), GetColorU32(ImGuiCol_TextDisabled));
        }

        const auto mouseInsideWidget = (IsItemActive() || IsItemHovered());
        const auto io = GetIO();
        const ImVec2 mouse = io.MousePos;
        assert(std::size(points) == std::size(handles));
        using ranges::views::zip, ranges::views::enumerate;
        if (mouseInsideWidget) {
            for (auto&& [i, pair] : enumerate(zip(points, handles))) {
                auto&& [point, handle] = pair;
                const auto distanceVector = mouse - handle;
                const auto squaredDistance = distanceVector.x * distanceVector.x + distanceVector.y * distanceVector.y;
                const auto mouseOverHandle = (squaredDistance < GRAB_RADIUS * GRAB_RADIUS);
                if (mouseOverHandle) {
                    SetTooltip("(%4.3f, %4.3f)", point.x, point.y);
                    if (IsMouseClicked(0)) {
                        *activeHandle = gsl::narrow_cast<int>(i);
                    }
                    break;
                }
            }
        }
        if (!IsMouseDown(0)) {
            *activeHandle = -1;
        }
        if (*activeHandle >= 0) {
            auto& point = points[*activeHandle];
            point.x += io.MouseDelta.x / Canvas.x;
            point.y -= io.MouseDelta.y / Canvas.y;
            if constexpr (AREA_CONSTRAINED) {
                point.x = std::clamp(point.x, 0.0f, 1.0f);
                point.y = std::clamp(point.y, 0.0f, 1.0f);
            }
            changed = true;
        }
        points[0].x = 0.0f;
        points[3].x = 1.0f;

        curve->leftY = points[0].y;
        curve->p0.x = points[1].x;
        curve->p0.y = points[1].y;
        curve->p1.x = points[2].x;
        curve->p1.y = points[2].y;
        curve->rightY = points[3].y;

        // eval curve
        const std::array pivotPoints = { ImVec2{ 0.0f, curve->leftY }, ImVec2{ curve->p0.x, curve->p0.y },
                                         ImVec2{ curve->p1.x, curve->p1.y }, ImVec2{ 1.0f, curve->rightY } };
        std::array<ImVec2, SMOOTHNESS + 1> results;
        bezier_table<SMOOTHNESS>(pivotPoints, results);

        // draw curve
        {
            ImColor color(GetStyle().Colors[ImGuiCol_PlotLines]);
            for (int i = 0; i < SMOOTHNESS; ++i) {
                ImVec2 p = { results[i + 0].x, 1 - results[i + 0].y };
                ImVec2 q = { results[i + 1].x, 1 - results[i + 1].y };
                ImVec2 r(p.x * (bb.Max.x - bb.Min.x) + bb.Min.x, p.y * (bb.Max.y - bb.Min.y) + bb.Min.y);
                ImVec2 s(q.x * (bb.Max.x - bb.Min.x) + bb.Min.x, q.y * (bb.Max.y - bb.Min.y) + bb.Min.y);
                DrawList->AddLine(r, s, color, CURVE_WIDTH);
            }
        }

        // draw lines and grabbers
        const ImVec4 white(GetStyle().Colors[ImGuiCol_Text]);
        const auto newHandles = getHandles(points);
        const auto luma = mouseInsideWidget ? 0.5f : 1.0f;
        const ImVec4 pink(1.00f, 0.00f, 0.75f, luma);
        const ImVec4 cyan(0.00f, 0.75f, 1.00f, luma);
        DrawList->AddLine(newHandles[0], newHandles[1], ImColor(white), LINE_WIDTH);
        DrawList->AddLine(newHandles[2], newHandles[3], ImColor(white), LINE_WIDTH);
        const std::array colors = { pink, pink, cyan, cyan };
        for (auto&& [handle, color] : ranges::views::zip(handles, colors)) {
            DrawList->AddCircleFilled(handle, GRAB_RADIUS, ImColor(white));
            DrawList->AddCircleFilled(handle, GRAB_RADIUS - GRAB_BORDER, ImColor(color));
        }

        return changed;
    }

}// namespace ImGui