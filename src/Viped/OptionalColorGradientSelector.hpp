//
// Created by coder2k on 27.11.2021.
//

#pragma once

//
// Created by coder2k on 22.11.2021.
//

#pragma once

#include "Color.hpp"
#include "ImGuiUtils/ColorGradient.hpp"
#include <imgui.h>
#include <IncludeGLM.hpp>
#include <string>
#include <optional>

class OptionalColorGradientSelector {
public:
    using OptionalGradient = std::optional<c2k::ParticleSystemImpl::ColorGradient>;

public:
    OptionalColorGradientSelector(std::string header) : mHeader{ std::move(header) } { }

    void operator()(OptionalGradient& optionalGradient) {
        using namespace c2k;
        using namespace c2k::ParticleSystemImpl;
        if (ImGui::CollapsingHeader(mHeader.c_str())) {
            mEnabled = optionalGradient.has_value();
            auto enabledBefore = mEnabled;
            ImGui::Checkbox("enabled", &mEnabled);
            if (mEnabled && !enabledBefore) {
                optionalGradient = c2k::ParticleSystemImpl::ColorGradient{};
                mGradient = ImGradient{};
                auto& gradient = optionalGradient.value();
                gradient.colorGradient.resize(mGradient.getMarks().size());
                for (std::size_t i = 0; i < mGradient.getMarks().size(); ++i) {
                    gradient.colorGradient[i] = mGradient.getMarks()[i];
                }
            } else if (!mEnabled && enabledBefore) {
                optionalGradient.reset();
            }

            if (optionalGradient.has_value()) {
                auto& gradient = optionalGradient.value();
                mGradient.getMarks().clear();
                mGradient.getMarks().reserve(gradient.colorGradient.size());
                for (const auto& mark : gradient.colorGradient) {
                    mGradient.getMarks().emplace_back(mark);
                }
                ImGui::GradientEditor(&mGradient, mDraggingMark, mSelectedMark);
                gradient.colorGradient.resize(mGradient.getMarks().size());
                for (std::size_t i = 0; i < mGradient.getMarks().size(); ++i) {
                    gradient.colorGradient[i] = mGradient.getMarks()[i];
                }
            }
        }
    }

private:
    std::string mHeader;
    ImGradientMark* mDraggingMark{ nullptr };
    ImGradientMark* mSelectedMark{ nullptr };
    ImGradient mGradient{};
    ImGradient::Marks mImGradientMarks{};
    bool mEnabled;
};