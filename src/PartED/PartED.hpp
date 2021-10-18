//
// Created by coder2k on 09.10.2021.
//

#pragma once

#include <Application.hpp>
#include <GUID.hpp>
#include <AssetList.hpp>
#include <imgui.h>
#include <tl/expected.hpp>
#include <variant>

class PartED : public c2k::Application {
private:
    using c2k::Application::Application;

    void setup() noexcept override;
    void update() noexcept override;
    void renderImGui() noexcept override;

    tl::expected<std::monostate, std::string> onNewProjectClicked() noexcept;
    void renderTextureList() noexcept;
    void renderParticleSystemList() noexcept;
    void renderStatsWindow() const noexcept;

    void refreshParticleSystem(
            const c2k::AssetDescriptions::ParticleSystemDescription& particleSystemDescription) noexcept;

private:
    static constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

    c2k::GUID mTextureGUID;
    c2k::GUID mShaderProgramGUID;
    c2k::Entity mParticleEmitterEntity{ c2k::invalidEntity };
    c2k::AssetList mAssetList;
    bool mHasParticleSystemDescriptionBeenLoaded{ false };
    c2k::AssetDescriptions::ParticleSystemDescription mParticleSystemDescription;
};
