//
// Created by coder2k on 09.10.2021.
//

#pragma once

#include "TwoWayVariantSelector.hpp"
#include "FourWayVariantSelector.hpp"
#include "FourWayVariantSelectorVec2.hpp"
#include "ColorVariantSelector.hpp"
#include "OptionalColorGradientSelector.hpp"
#include "OptionalBezierCurveSelector.hpp"
#include <Application.hpp>
#include <GUID.hpp>
#include <AssetList.hpp>
#include <imgui.h>
#include <tl/expected.hpp>
#include <variant>
#include <limits>

class Viped : public c2k::Application {
private:
    enum class ParticleEmitterMovementPattern {
        None,
        HorizontalBounce,
        VerticalBounce,
        Circle,
    };

private:
    using c2k::Application::Application;

    void setup() noexcept override;
    void update() noexcept override;
    void renderImGui() noexcept override;

    tl::expected<std::monostate, std::string> onNewProjectClicked() noexcept;
    void renderMainMenu() noexcept;
    void renderTextureList() noexcept;
    void renderParticleSystemList() noexcept;
    void renderStatsWindow() const noexcept;
    void renderInspectorWindow() noexcept;
    void renderParticleEmitterWindow() noexcept;

    void refreshParticleSystem(
            const c2k::AssetDescriptions::ParticleSystemDescription& particleSystemDescription) noexcept;

    void changeTexture(const c2k::AssetDescriptions::TextureDescription& textureDescription) noexcept;
    void closeParticleSystem() noexcept;
    void saveParticleSystem() noexcept;

private:
    static constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

    c2k::GUID mTextureGUID;
    c2k::GUID mShaderProgramGUID;
    c2k::Entity mParticleEmitterEntity{ c2k::invalidEntity };
    c2k::AssetList mAssetList;
    c2k::AssetDescriptions::ParticleSystemDescription mParticleSystemDescription;
    c2k::ParticleSystem* mParticleSystem{ nullptr };

    FourWayVariantSelector<double> mStartLifeTimeSelector{ "Start Lifetime", 0.05, 0.0, 60.0 };
    TwoWayVariantSelector<double> mStartDelaySelector{ "Start Delay", 0.05, 0.0, std::numeric_limits<double>::max() };
    FourWayVariantSelector<float> mStartSpeedSelector{ "Start Speed", 0.05f, std::numeric_limits<float>::lowest(),
                                                       std::numeric_limits<float>::max() };
    FourWayVariantSelectorVec2 mStartSizeSelector{ "Start Size", 0.05f, 0.0f, std::numeric_limits<float>::max() };
    ParticleEmitterMovementPattern mParticleEmitterMovementPattern{ ParticleEmitterMovementPattern::None };
    float mParticleEmitterMovementRadius{ 200.0f };
    double mParticleEmitterMovementSpeedFactor{ 1.0 };
    FourWayVariantSelector<float> mStartRotationSelector{ "Start Rotation", 0.05f, 0.0f, 360.0f };
    ColorVariantSelector mColorSelector{ "Color" };
    FourWayVariantSelector<float> mGravityModifierSelector{ "Gravity Modifier", 0.05f,
                                                            std::numeric_limits<float>::lowest(),
                                                            std::numeric_limits<float>::max() };
    FourWayVariantSelector<float> mRateOverTimeSelector{ "Rate Over Time", 0.05f, std::numeric_limits<float>::min(),
                                                         std::numeric_limits<float>::max() };
    FourWayVariantSelectorVec2 mLinearVelocityOverLifetimeSelector{ "Linear Velocity Over Lifetime", 0.05f,
                                                                    std::numeric_limits<float>::lowest(),
                                                                    std::numeric_limits<float>::max() };
    FourWayVariantSelector<float> mRadialVelocityOverLifetimeSelector{ "Radial Velocity Over Lifetime", 0.05f,
                                                                       std::numeric_limits<float>::lowest(),
                                                                       std::numeric_limits<float>::max() };
    OptionalColorGradientSelector mColorOverLifetimeSelector{ "Color Over Lifetime" };
    OptionalBezierCurveSelector mSizeOverLifetimeSelector{ "Size Over Lifetime", 0.05f,
                                                           std::numeric_limits<float>::lowest(),
                                                           std::numeric_limits<float>::max() };
};
