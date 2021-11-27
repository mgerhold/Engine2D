//
// Created by coder2k on 01.11.2021.
//

#pragma once

#include "Utility.hpp"
#include "GUID.hpp"
#include "ImGuiUtils/Bezier.hpp"
#include <ParticleSystem.hpp>
#include <imgui.h>
#include <variant>
#include <string>

template<typename T>
class FourWayVariantSelector {
public:
    using FourWayVariant = std::variant<T,
                                        c2k::ParticleSystemImpl::Range<T>,
                                        c2k::ParticleSystemImpl::BezierCurve,
                                        c2k::ParticleSystemImpl::Range<c2k::ParticleSystemImpl::BezierCurve>>;

public:
    FourWayVariantSelector(std::string header, T speed, T min, T max) noexcept
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
                if (holds_alternative<Range<T>>(variant)) {
                    variant = get<Range<T>>(variant).min;
                } else if (!holds_alternative<T>(variant)) {
                    variant = T{ 1 };
                }
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Range", &mCurrentlySelected, 1)) {
                if (holds_alternative<T>(variant)) {
                    variant = Range<T>{ .min{ get<T>(variant) }, .max{ get<T>(variant) } };
                } else if (!holds_alternative<Range<T>>(variant)) {
                    variant = Range<T>{ .min{ 1.0 }, .max{ 1.0 } };
                }
            }
            if (ImGui::RadioButton("Curve", &mCurrentlySelected, 2)) {
                if (!holds_alternative<BezierCurve>(variant)) {
                    variant = BezierCurve{ .p0{ 0.25f, 0.25f },
                                           .p1{ 0.75f, 0.75f },
                                           .minVal{ 0.0 },
                                           .maxVal{ 2.0 },
                                           .leftY{ 0.0f },
                                           .rightY{ 1.0f } };
                }
            }
            ImGui::SameLine();
            ImGui::RadioButton("Random Between Curves", &mCurrentlySelected, 3);
            switch (mCurrentlySelected) {
                case 0:
                    // constant
                    drag("value", &get<T>(variant), mSpeed, mMin, mMax);
                    break;
                case 1:
                    // range
                    drag("min", &(get<Range<T>>(variant).min), mSpeed, mMin, get<Range<T>>(variant).max);
                    drag("max", &(get<Range<T>>(variant).max), mSpeed, get<Range<T>>(variant).min, mMax);
                    break;
                case 2:
                    // curve
                    auto& curve = get<BezierCurve>(variant);
                    ImGui::Bezier("curve", &curve, &mActiveCurveHandle0, gsl::narrow_cast<float>(mSpeed),
                                  gsl::narrow_cast<float>(mMin), gsl::narrow_cast<float>(mMax));
                    break;
            }
        }
        ImGui::PopID();
    }

private:
    const std::string mHeader;
    const std::string mID;
    int mCurrentlySelected;
    const T mSpeed;
    const T mMin;
    const T mMax;
    int mActiveCurveHandle0{ -1 };
};