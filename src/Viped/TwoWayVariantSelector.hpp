//
// Created by coder2k on 14.11.2021.
//

#pragma once

#include "Utility.hpp"
#include "ImGuiUtils/Bezier.hpp"
#include <ParticleSystem.hpp>
#include <imgui.h>
#include <variant>
#include <string>

template<typename T>
class TwoWayVariantSelector {
public:
    using TwoWayVariant = std::variant<T, c2k::ParticleSystemImpl::Range<T>>;

public:
    TwoWayVariantSelector(std::string header, T speed, T min, T max) noexcept
        : mHeader{ std::move(header) },
          mID { c2k::GUID::create().string() },
          mCurrentlySelected{ 0 },
          mSpeed{ speed },
          mMin{ min },
          mMax{ max } { }

    void operator()(TwoWayVariant& variant) noexcept {
        using namespace c2k::ParticleSystemImpl;
        ImGui::PushID(mID.c_str());
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
            }
        }
        ImGui::PopID();
    }

private:
    const std::string mHeader;
    const std::string mID;
    int mCurrentlySelected;
    const double mSpeed;
    const double mMin;
    const double mMax;
};