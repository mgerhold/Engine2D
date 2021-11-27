//
// Created by coder2k on 01.11.2021.
//

#pragma once

#include "Utility.hpp"
#include "GUID.hpp"
#include "ImGuiUtils/Bezier.hpp"
#include <ParticleSystem.hpp>
#pragma warning(push)
#pragma warning(disable: 4201)
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#pragma warning(pop)
#include <imgui.h>
#include <variant>
#include <string>

class FourWayVariantSelectorVec2 {
public:
    using FourWayVariant = std::variant<glm::vec2,
                                        c2k::ParticleSystemImpl::Range<glm::vec2>,
                                        c2k::ParticleSystemImpl::BezierCurves2D,
                                        c2k::ParticleSystemImpl::Range<c2k::ParticleSystemImpl::BezierCurves2D>>;

public:
    FourWayVariantSelectorVec2(std::string header, float speed, float min, float max) noexcept
        : mHeader{ std::move(header) },
          mID{ c2k::GUID::create().string() },
          mCurrentlySelected{ 0 },
          mSpeed{ speed },
          mMin{ min },
          mMax{ max } { }

    void operator()(FourWayVariant& variant) noexcept {
        using namespace c2k::ParticleSystemImpl;
        ImGui::PushID(mID.c_str());
        if (ImGui::CollapsingHeader(mHeader.c_str())) {
            if (mCurrentlySelected != gsl::narrow_cast<int>(variant.index())) {
                mCurrentlySelected = gsl::narrow_cast<int>(variant.index());
            }
            if (ImGui::RadioButton("Constant", &mCurrentlySelected, 0)) {
                if (holds_alternative<Range<glm::vec2>>(variant)) {
                    variant = get<Range<glm::vec2>>(variant).min;
                } else if (!holds_alternative<glm::vec2>(variant)) {
                    variant = glm::vec2{ 1.0f };
                }
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Range", &mCurrentlySelected, 1)) {
                if (holds_alternative<glm::vec2>(variant)) {
                    variant = Range<glm::vec2>{ .min{ get<glm::vec2>(variant) }, .max{ get<glm::vec2>(variant) } };
                } else if (!holds_alternative<Range<glm::vec2>>(variant)) {
                    variant = Range<glm::vec2>{ .min{ glm::vec2{ 1.0 } }, .max{ glm::vec2{  1.0 } } };
                }
            }
            if (ImGui::RadioButton("Curve", &mCurrentlySelected, 2)) {
                if (!holds_alternative<BezierCurves2D>(variant)) {
                    variant = BezierCurves2D{ .x{ .p0{ 0.25f, 0.25f },
                                                  .p1{ 0.75f, 0.75f },
                                                  .minVal{ 0.0 },
                                                  .maxVal{ 2.0 },
                                                  .leftY{ 0.0f },
                                                  .rightY{ 1.0f } },
                                              .y{ .p0{ 0.25f, 0.25f },
                                                  .p1{ 0.75f, 0.75f },
                                                  .minVal{ 0.0 },
                                                  .maxVal{ 2.0 },
                                                  .leftY{ 0.0f },
                                                  .rightY{ 1.0f } } };
                }
            }
            ImGui::SameLine();
            ImGui::RadioButton("Random Between Curves", &mCurrentlySelected, 3);
            switch (mCurrentlySelected) {
                case 0:
                    // constant
                    ImGui::Checkbox("Uniform", &mConstantUniform);
                    if (mConstantUniform) {
                        ImGui::DragFloat("value", &get<glm::vec2>(variant).x, mSpeed, mMin, mMax);
                        get<glm::vec2>(variant).y = get<glm::vec2>(variant).x;
                    } else {
                        ImGui::DragFloat2("value", glm::value_ptr(get<glm::vec2>(variant)), mSpeed, mMin, mMax);
                    }
                    break;
                case 1:
                    // range
                    ImGui::Checkbox("Uniform", &mRangeUniform);
                    if (mRangeUniform) {
                        ImGui::DragFloat("min", &get<Range<glm::vec2>>(variant).min.x, mSpeed, mMin, mMax);
                        get<Range<glm::vec2>>(variant).min.y = get<Range<glm::vec2>>(variant).min.x;
                        ImGui::DragFloat("max", &get<Range<glm::vec2>>(variant).max.x, mSpeed, mMin, mMax);
                        get<Range<glm::vec2>>(variant).max.y = get<Range<glm::vec2>>(variant).max.x;
                    } else {
                        ImGui::DragFloat2("min", glm::value_ptr(get<Range<glm::vec2>>(variant).min), mSpeed, mMin,
                                          mMax);
                        ImGui::DragFloat2("max", glm::value_ptr(get<Range<glm::vec2>>(variant).max), mSpeed, mMin,
                                          mMax);
                    }
                    break;
                case 2:
                    // curve
                    ImGui::Checkbox("Uniform", &mCurveUniform);
                    if (mCurveUniform) {
                        auto& curves = get<BezierCurves2D>(variant);
                        ImGui::Bezier("curve", &curves.x, &mActiveCurveHandle0, mSpeed);
                        curves.y = curves.x;
                    } else {
                        auto& curves = get<BezierCurves2D>(variant);
                        ImGui::PushID("_curveX_");
                        ImGui::Bezier("curve", &curves.x, &mActiveCurveHandle0, mSpeed);
                        ImGui::PopID();
                        ImGui::PushID("_curveY_");
                        ImGui::Bezier("curve", &curves.y, &mActiveCurveHandle0, mSpeed);
                        ImGui::PopID();
                    }
                    break;
            }
        }
        ImGui::PopID();
    }

private:
    const std::string mHeader;
    const std::string mID;
    int mCurrentlySelected;
    const float mSpeed;
    const float mMin;
    const float mMax;
    int mActiveCurveHandle0{ -1 };
    bool mConstantUniform{ true };
    bool mRangeUniform{ true };
    bool mCurveUniform{ true };
};