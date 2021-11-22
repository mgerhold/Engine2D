//
// Created by coder2k on 22.11.2021.
//

#pragma once

// https://gist.github.com/galloscript/8a5d179e432e062550972afcd1ecf112#file-imgui_color_gradient-cpp

//
//  imgui_color_gradient.h
//  imgui extension
//
//  Created by David Gallardo on 11/06/16.

/*

 Usage:

 ::GRADIENT DATA::
 ImGradient gradient;

 ::BUTTON::
 if(ImGui::GradientButton(&gradient))
 {
    //set show editor flag to true/false
 }

 ::EDITOR::
 static ImGradientMark* draggingMark = nullptr;
 static ImGradientMark* selectedMark = nullptr;

 bool updated = ImGui::GradientEditor(&gradient, draggingMark, selectedMark);

 ::GET A COLOR::
 float color[3];
 gradient.getColorAt(0.3f, color); //position from 0 to 1

 ::MODIFY GRADIENT WITH CODE::
 gradient.getMarks().clear();
 gradient.addMark(0.0f, ImColor(0.2f, 0.1f, 0.0f));
 gradient.addMark(0.7f, ImColor(120, 200, 255));

 ::WOOD BROWNS PRESET::
 gradient.getMarks().clear();
 gradient.addMark(0.0f, ImColor(0xA0, 0x79, 0x3D));
 gradient.addMark(0.2f, ImColor(0xAA, 0x83, 0x47));
 gradient.addMark(0.3f, ImColor(0xB4, 0x8D, 0x51));
 gradient.addMark(0.4f, ImColor(0xBE, 0x97, 0x5B));
 gradient.addMark(0.6f, ImColor(0xC8, 0xA1, 0x65));
 gradient.addMark(0.7f, ImColor(0xD2, 0xAB, 0x6F));
 gradient.addMark(0.8f, ImColor(0xDC, 0xB5, 0x79));
 gradient.addMark(1.0f, ImColor(0xE6, 0xBF, 0x83));

 */

#include <Color.hpp>
#include <ParticleSystem.hpp>
#include <JSON/JSON.hpp>
#include <imgui.h>
#include <list>
#include <memory>
#include <vector>
#include <array>


struct ImGradientMark {
    ImGradientMark(ImColor color, float position) : color{ color }, position{ position } { }

    ImGradientMark(const c2k::ParticleSystemImpl::ColorGradientMark& mark)
        : color{ ImColor{ mark.color.r, mark.color.g, mark.color.b, mark.color.a } },
          position{ mark.position } { }

    operator c2k::ParticleSystemImpl::ColorGradientMark() const {
        c2k::ParticleSystemImpl::ColorGradientMark result;
        result.color = c2k::Color{ color.Value.x, color.Value.y, color.Value.z, color.Value.w };
        result.position = position;
        return result;
    }

    ImColor color;
    float position;//0 to 1
};

class ImGradient {
public:
    using Marks = std::vector<ImGradientMark>;

public:
    ImGradient();

    ImColor getColorAt(float position) const;
    void addMark(float position, ImColor const color);
    void removeMark(Marks::iterator iterator);
    void refreshCache();
    Marks& getMarks() {
        return mMarks;
    }

private:
    using MarkIteratorPair = std::pair<Marks::const_iterator, Marks::const_iterator>;

private:
    MarkIteratorPair findBounds(float position) const;
    static ImColor lerpColor(const ImColor& lhs, const ImColor& rhs, float interpolationParameter);

private:
    ImColor computeColorAt(float position) const;
    Marks mMarks;
    std::array<ImColor, 256> mCachedValues;
};

namespace ImGui {
    bool GradientButton(ImGradient* gradient);

    bool GradientEditor(ImGradient* gradient, ImGradientMark*& draggingMark, ImGradientMark*& selectedMark);
}// namespace ImGui
