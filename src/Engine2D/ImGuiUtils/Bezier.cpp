#include "Bezier.hpp"
#include <gsl/gsl>
#include <array>
#include <limits>

namespace ImGui {

    template<int steps>
    void bezier_table(ImVec2 P[], ImVec2 results[]) {
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
            ImVec2 point = { K[step * 4 + 0] * P[0].x + K[step * 4 + 1] * P[1].x + K[step * 4 + 2] * P[2].x +
                                     K[step * 4 + 3] * P[3].x,
                             K[step * 4 + 0] * P[0].y + K[step * 4 + 1] * P[1].y + K[step * 4 + 2] * P[2].y +
                                     K[step * 4 + 3] * P[3].y };
            results[step] = point;
        }
    }

    float BezierValue(float dt01, glm::vec2 p0, glm::vec2 p1, float minVal, float maxVal) {
        constexpr auto STEPS = 256;
        ImVec2 Q[4] = { { 0.0f, 0.0f }, { p0.x, p0.y }, { p1.x, p1.y }, { 1.0f, 1.0f } };
        ImVec2 results[STEPS + 1];
        bezier_table<STEPS>(Q, results);
        return results[(int) ((dt01 < 0 ? 0 : dt01 > 1 ? 1 : dt01) * STEPS)].y * (maxVal - minVal) + minVal;
    }

    int Bezier(const char* label,
               glm::vec2* p0,
               glm::vec2* p1,
               float* minVal,
               float* maxVal,
               int* selectedPreset,
               float speed) {
        // visuals
        constexpr auto SMOOTHNESS = 64;        // curve smoothness: the higher number of segments, the smoother curve
        constexpr auto CURVE_WIDTH = 4;        // main curved line width
        constexpr auto LINE_WIDTH = 1;         // handlers: small lines width
        constexpr auto GRAB_RADIUS = 8;        // handlers: circle radius
        constexpr auto GRAB_BORDER = 2;        // handlers: circle border width
        constexpr auto AREA_CONSTRAINED = true;// should grabbers be constrained to grid area?
        constexpr auto AREA_WIDTH = 128;       // area width in pixels. 0 for adaptive size (will use max avail width)

        // curve presets
        struct CurvePreset {
            const char* name;
            glm::vec2 p0, p1;
        };
        constexpr CurvePreset presets[] = {
            { "Linear", { 0.000f, 0.000f }, { 1.000f, 1.000f } },

            { "In Sine", { 0.470f, 0.000f }, { 0.745f, 0.715f } },
            { "In Quad", { 0.550f, 0.085f }, { 0.680f, 0.530f } },
            { "In Cubic", { 0.550f, 0.055f }, { 0.675f, 0.190f } },
            { "In Quart", { 0.895f, 0.030f }, { 0.685f, 0.220f } },
            { "In Quint", { 0.755f, 0.050f }, { 0.855f, 0.060f } },
            { "In Expo", { 0.950f, 0.050f }, { 0.795f, 0.035f } },
            { "In Circ", { 0.600f, 0.040f }, { 0.980f, 0.335f } },
            { "In Back", { 0.600f, -0.28f }, { 0.735f, 0.045f } },

            { "Out Sine", { 0.390f, 0.575f }, { 0.565f, 1.000f } },
            { "Out Quad", { 0.250f, 0.460f }, { 0.450f, 0.940f } },
            { "Out Cubic", { 0.215f, 0.610f }, { 0.355f, 1.000f } },
            { "Out Quart", { 0.165f, 0.840f }, { 0.440f, 1.000f } },
            { "Out Quint", { 0.230f, 1.000f }, { 0.320f, 1.000f } },
            { "Out Expo", { 0.190f, 1.000f }, { 0.220f, 1.000f } },
            { "Out Circ", { 0.075f, 0.820f }, { 0.165f, 1.000f } },
            { "Out Back", { 0.175f, 0.885f }, { 0.320f, 1.275f } },

            { "InOut Sine", { 0.445f, 0.050f }, { 0.550f, 0.950f } },
            { "InOut Quad", { 0.455f, 0.030f }, { 0.515f, 0.955f } },
            { "InOut Cubic", { 0.645f, 0.045f }, { 0.355f, 1.000f } },
            { "InOut Quart", { 0.770f, 0.000f }, { 0.175f, 1.000f } },
            { "InOut Quint", { 0.860f, 0.000f }, { 0.070f, 1.000f } },
            { "InOut Expo", { 1.000f, 0.000f }, { 0.000f, 1.000f } },
            { "InOut Circ", { 0.785f, 0.135f }, { 0.150f, 0.860f } },
            { "InOut Back", { 0.680f, -0.55f }, { 0.265f, 1.550f } },

            // easeInElastic: not a bezier
            // easeOutElastic: not a bezier
            // easeInOutElastic: not a bezier
            // easeInBounce: not a bezier
            // easeOutBounce: not a bezier
            // easeInOutBounce: not a bezier
        };

        // preset selector

        bool loadPreset = false;
        ImGui::PushID(label);
        if (ImGui::ArrowButton("##lt", ImGuiDir_Left)) {
            if (*selectedPreset > 0) {
                --(*selectedPreset);
                loadPreset = true;
            }
        }
        ImGui::SameLine();

        if (ImGui::Button("Presets")) {
            ImGui::OpenPopup("!Presets");
        }
        if (ImGui::BeginPopup("!Presets")) {
            for (int i = 0; i < std::size(presets); ++i) {
                if (i == 1 || i == 9 || i == 17)
                    ImGui::Separator();
                if (ImGui::MenuItem(presets[i].name, nullptr, *selectedPreset == i)) {
                    *selectedPreset = i;
                    loadPreset = true;
                }
            }
            ImGui::EndPopup();
        }
        ImGui::SameLine();

        if (ImGui::ArrowButton("##rt", ImGuiDir_Right)) {
            if (*selectedPreset < std::size(presets) - 1) {
                ++(*selectedPreset);
                loadPreset = true;
            }
        }
        // ImGui::SameLine();
        ImGui::PopID();

        if (loadPreset) {
            *p0 = presets[*selectedPreset].p0;
            *p1 = presets[*selectedPreset].p1;
        }

        // bezier widget
        const ImGuiStyle& Style = GetStyle();
        // const ImGuiIO& IO = GetIO();
        ImDrawList* DrawList = GetWindowDrawList();
        ImGuiWindow* Window = GetCurrentWindow();
        if (Window->SkipItems)
            return false;

        // header and spacing
        bool changed = false;
        ImGui::Text("(%0.2f, %0.2f), (%0.2f, %0.2f)", p0->x, p0->y, p1->x, p1->y);
        ImGui::DragFloat("min", minVal, speed, 0.0f, std::numeric_limits<float>::max());
        ImGui::DragFloat("max", maxVal, speed, 0.0f, std::numeric_limits<float>::max());

        bool hovered = IsItemActive() || IsItemHovered();
        Dummy(ImVec2(0, 3));

        // prepare canvas
        const float avail = GetContentRegionAvail().x;
        const float dim = AREA_WIDTH > 0 ? AREA_WIDTH : avail;
        ImVec2 Canvas(dim, dim);

        ImRect bb(Window->DC.CursorPos, Window->DC.CursorPos + Canvas);
        ItemSize(bb);
        if (!ItemAdd(bb, 0)) {
            return changed;
        }

        const ImGuiID id = Window->GetID(label);
        hovered |= 0 != ItemHoverable(ImRect(bb.Min, bb.Min + ImVec2(avail, dim)), id);

        RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg, 1), true, Style.FrameRounding);

        // background grid
        for (int i = 0; i <= Canvas.x; i += gsl::narrow_cast<int>(Canvas.x / 4.0f)) {
            DrawList->AddLine(ImVec2(bb.Min.x + i, bb.Min.y), ImVec2(bb.Min.x + i, bb.Max.y),
                              GetColorU32(ImGuiCol_TextDisabled));
        }
        for (int i = 0; i <= Canvas.y; i += gsl::narrow_cast<int>(Canvas.y / 4.0f)) {
            DrawList->AddLine(ImVec2(bb.Min.x, bb.Min.y + i), ImVec2(bb.Max.x, bb.Min.y + i),
                              GetColorU32(ImGuiCol_TextDisabled));
        }

        // eval curve
        ImVec2 Q[4] = { { 0, 0 }, { p0->x, p0->y }, { p1->x, p1->y }, { 1, 1 } };
        ImVec2 results[SMOOTHNESS + 1];
        bezier_table<SMOOTHNESS>(Q, results);

        // control points: 2 lines and 2 circles
        {
            // handle grabbers
            ImVec2 mouse = GetIO().MousePos;
            const glm::vec2 mousePos{ mouse.x, mouse.y };
            const auto rectMax = glm::vec2{ bb.Max.x, bb.Max.y };
            const auto rectMin = glm::vec2{ bb.Min.x, bb.Min.y };
            const auto pos0 = (glm::vec2{ p0->x, 1.0f - p0->y } * (rectMax - rectMin)) + rectMin;
            const auto pos1 = (glm::vec2{ p1->x, 1.0f - p1->y } * (rectMax - rectMin)) + rectMin;
            std::array<float, 2> distances = { glm::dot(pos0 - mousePos, pos0 - mousePos),
                                               glm::dot(pos1 - mousePos, pos1 - mousePos) };

            const int selectedIndex = distances[0] < distances[1] ? 0 : 1;
            auto& selectedPoint = (selectedIndex == 0 ? *p0 : *p1);
            if (distances[selectedIndex] < (4 * GRAB_RADIUS * 4 * GRAB_RADIUS)) {
                SetTooltip("(%4.3f, %4.3f)", selectedPoint.x, selectedPoint.y);
                if (IsMouseClicked(0) || IsMouseDragging(0)) {
                    float canvasScale = 1.0f / ImGui::GetFontSize();
                    selectedPoint.x += GetIO().MouseDelta.x / Canvas.x;
                    selectedPoint.y -= GetIO().MouseDelta.y / Canvas.y;
                    if constexpr (AREA_CONSTRAINED) {
                        selectedPoint.x = (selectedPoint.x < 0 ? 0 : (selectedPoint.x > 1 ? 1 : selectedPoint.x));
                        selectedPoint.y = (selectedPoint.y < 0 ? 0 : (selectedPoint.y > 1 ? 1 : selectedPoint.y));
                    }
                    changed = true;
                }
            }
        }

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
        ImVec4 white(GetStyle().Colors[ImGuiCol_Text]);
        float luma = IsItemActive() || IsItemHovered() ? 0.5f : 1.0f;
        const ImVec4 pink(1.00f, 0.00f, 0.75f, luma), cyan(0.00f, 0.75f, 1.00f, luma);
        const ImVec2 _p1 = ImVec2(p0->x, 1 - p0->y) * (bb.Max - bb.Min) + bb.Min;
        const ImVec2 _p2 = ImVec2(p1->x, 1 - p1->y) * (bb.Max - bb.Min) + bb.Min;
        DrawList->AddLine(ImVec2(bb.Min.x, bb.Max.y), _p1, ImColor(white), LINE_WIDTH);
        DrawList->AddLine(ImVec2(bb.Max.x, bb.Min.y), _p2, ImColor(white), LINE_WIDTH);
        DrawList->AddCircleFilled(_p1, GRAB_RADIUS, ImColor(white));
        DrawList->AddCircleFilled(_p1, GRAB_RADIUS - GRAB_BORDER, ImColor(pink));
        DrawList->AddCircleFilled(_p2, GRAB_RADIUS, ImColor(white));
        DrawList->AddCircleFilled(_p2, GRAB_RADIUS - GRAB_BORDER, ImColor(cyan));

        return changed;
    }

}// namespace ImGui