//
// Created by coder2k on 01.11.2021.
//

#pragma once

#include "Utility.hpp"
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
          mCurrentlySelected{ 0 },
          mSpeed{ speed },
          mMin{ min },
          mMax{ max } { }

    void operator()(FourWayVariant& variant) noexcept {
        using namespace c2k::ParticleSystemImpl;
        if (ImGui::CollapsingHeader(mHeader.c_str())) {
            if (mCurrentlySelected != gsl::narrow_cast<int>(variant.index())) {
                mCurrentlySelected = gsl::narrow_cast<int>(variant.index());
            }
            if (ImGui::RadioButton("Constant", &mCurrentlySelected, 0)) {
                if (holds_alternative<Range<double>>(variant)) {
                    variant = get<Range<double>>(variant).min;
                } else if (!holds_alternative<double>(variant)) {
                    variant = 1.0;
                }
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Range", &mCurrentlySelected, 1)) {
                if (holds_alternative<double>(variant)) {
                    variant = Range<double>{ .min{ get<double>(variant) }, .max{ get<double>(variant) } };
                } else if (!holds_alternative<Range<double>>(variant)) {
                    variant = Range<double>{ .min{ 1.0 }, .max{ 1.0 } };
                }
            }
            if (ImGui::RadioButton("Curve", &mCurrentlySelected, 2)) {
                if (!holds_alternative<BezierCurve>(variant)) {
                    variant = BezierCurve{ .p0{ 0.25f, 0.25f }, .p1{ 0.75f, 0.75f }, .minVal{ 0.0 }, .maxVal{ 2.0 } };
                }
            }
            ImGui::SameLine();
            ImGui::RadioButton("Random Between Curves", &mCurrentlySelected, 3);
            switch (mCurrentlySelected) {
                case 0:
                    // constant
                    dragDouble("value", &get<double>(variant), mSpeed, mMin, mMax);
                    break;
                case 1:
                    // range
                    dragDouble("min", &(get<Range<double>>(variant).min), mSpeed, mMin,
                               get<Range<double>>(variant).max);
                    dragDouble("max", &(get<Range<double>>(variant).max), mSpeed, get<Range<double>>(variant).min,
                               mMax);
                    break;
                case 2:
                    // curve
                    auto& curve = get<BezierCurve>(variant);
                    ImGui::Bezier("curve", &curve.p0, &curve.p1, &curve.minVal, &curve.maxVal, &mCurvePreset0,
                                  gsl::narrow_cast<float>(mSpeed));
                    break;
            }
        }
    }

private:
    const std::string mHeader;
    int mCurrentlySelected;
    const double mSpeed;
    const double mMin;
    const double mMax;
    int mCurvePreset0{ 0 };
};