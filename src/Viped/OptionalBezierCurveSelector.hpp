//
// Created by coder2k on 29.11.2021.
//

#pragma once

//
// Created by coder2k on 01.11.2021.
//

#pragma once

#include "Utility.hpp"
#include "GUID.hpp"
#include "ImGuiUtils/Bezier.hpp"
#include <ParticleSystem.hpp>
#include <imgui.h>
#include <optional>
#include <string>

class OptionalBezierCurveSelector {
public:
    using OptionalBezierCurve = std::optional<c2k::ParticleSystemImpl::BezierCurve>;

public:
    OptionalBezierCurveSelector(std::string header, float speed, float min, float max) noexcept
        : mHeader{ std::move(header) },
          mID{ c2k::GUID::create().string() },
          mSpeed{ speed },
          mMin{ min },
          mMax{ max } { }

    void operator()(OptionalBezierCurve& optionalCurve) noexcept {
        using namespace c2k::ParticleSystemImpl;
        ImGui::PushID(mID.c_str());
        if (ImGui::CollapsingHeader(mHeader.c_str())) {
            mEnabled = optionalCurve.has_value();
            const auto enabledBefore = mEnabled;
            ImGui::Checkbox("enabled", &mEnabled);
            if (mEnabled && !enabledBefore) {
                optionalCurve = BezierCurve{ .p0{ 0.25f, 0.25f },
                                             .p1{ 0.75f, 0.75f },
                                             .minVal{ 0.0 },
                                             .maxVal{ 2.0 },
                                             .leftY{ 0.0f },
                                             .rightY{ 1.0f } };
            } else if (!mEnabled && enabledBefore) {
                optionalCurve.reset();
            }
            if (optionalCurve.has_value()) {
                ImGui::Bezier("curve", &optionalCurve.value(), &mActiveCurveHandle0, gsl::narrow_cast<float>(mSpeed),
                              gsl::narrow_cast<float>(mMin), gsl::narrow_cast<float>(mMax));
            }
        }
        ImGui::PopID();
    }

private:
    const std::string mHeader;
    const std::string mID;
    bool mEnabled{ false };
    const float mSpeed;
    const float mMin;
    const float mMax;
    int mActiveCurveHandle0{ -1 };
};