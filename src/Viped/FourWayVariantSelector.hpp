//
// Created by coder2k on 01.11.2021.
//

#pragma once

#include "Utility.hpp"
#include <ParticleSystem.hpp>
#include <imgui.h>
#include <variant>
#include <string>

template<typename T>
class FourWayVariantSelector {
public:
    using FourWayVariant = std::variant<T,
                                        c2k::ParticleSystemImpl::Range<T>,
                                        c2k::ParticleSystemImpl::BezierSpline,
                                        c2k::ParticleSystemImpl::Range<c2k::ParticleSystemImpl::BezierSpline>>;

public:
    FourWayVariantSelector(std::string header, double speed, double min, double max) noexcept
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
            ImGui::RadioButton("Curve", &mCurrentlySelected, 2);
            ImGui::SameLine();
            ImGui::RadioButton("Random Between Curves", &mCurrentlySelected, 3);
            switch (mCurrentlySelected) {
                case 0:
                    dragDouble("value", &get<double>(variant), mSpeed, mMin, mMax);
                    break;
                case 1:
                    dragDouble("min", &(get<Range<double>>(variant).min), mSpeed, mMin,
                               get<Range<double>>(variant).max);
                    dragDouble("max", &(get<Range<double>>(variant).max), mSpeed, get<Range<double>>(variant).min,
                               mMax);
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
};