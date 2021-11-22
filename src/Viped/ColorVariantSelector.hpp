//
// Created by coder2k on 22.11.2021.
//

#pragma once

#include "Color.hpp"
#include "ImGuiUtils/ColorGradient.hpp"
#include <imgui.h>
#pragma warning(push)
#pragma warning(disable : 4201)
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#pragma warning(pop)
#include <string>

class ColorVariantSelector {
private:
    using ColorVariant = std::variant<c2k::Color, c2k::ParticleSystemImpl::ColorGradient>;

public:
    ColorVariantSelector(std::string header) : mHeader{ std::move(header) } { }

    void operator()(ColorVariant& variant) {
        using namespace c2k;
        using namespace c2k::ParticleSystemImpl;
        if (ImGui::CollapsingHeader(mHeader.c_str())) {
            mCurrentlySelected =
                    static_cast<int>(mCurrentlySelected == variant.index() ? mCurrentlySelected : variant.index());
            if (ImGui::RadioButton("Constant", &mCurrentlySelected, 0) && holds_alternative<ColorGradient>(variant)) {
                variant = Color::white();
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Gradient", &mCurrentlySelected, 1) && holds_alternative<Color>(variant)) {
                variant = ColorGradient{};
                mGradient = ImGradient{};
                auto& gradient = get<ColorGradient>(variant);
                gradient.colorGradient.resize(mGradient.getMarks().size());
                for (std::size_t i = 0; i < mGradient.getMarks().size(); ++i) {
                    gradient.colorGradient[i] = mGradient.getMarks()[i];
                }
            }

            if (holds_alternative<Color>(variant)) {
                constexpr ImGuiColorEditFlags flags = ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaBar;
                ImGui::ColorPicker4("Color", glm::value_ptr(static_cast<glm::vec4&>(get<Color>(variant))), flags);
            } else if (holds_alternative<ColorGradient>(variant)) {
                auto& gradient = get<ColorGradient>(variant);
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
    int mCurrentlySelected{ 0 };
    ImGradientMark* mDraggingMark{ nullptr };
    ImGradientMark* mSelectedMark{ nullptr };
    ImGradient mGradient{};
    ImGradient::Marks mImGradientMarks{};
};