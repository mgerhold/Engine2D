//
// Created by coder2k on 09.10.2021.
//

#include "PartED.hpp"
#include "AssetDatabase.hpp"
#include "Component.hpp"
#include "Platform.hpp"
#include <imgui.h>
#include <tinyfiledialogs/tinyfiledialogs.h>
#include <spdlog/spdlog.h>
#include <gsl/gsl>
#include <array>
#include <filesystem>
#include <iostream>

void PartED::setup() noexcept {
    mTextureGUID = c2k::GUID::create();      // initialize to a yet unknown GUID
    mShaderProgramGUID = c2k::GUID::create();// same
}

void PartED::update() noexcept { }

void PartED::renderImGui() noexcept {
    using namespace c2k;
    bool openErrorPopup = false;
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open...")) {
                const auto result =
                        openFileDialog("Open File", c2k::AssetDatabase::assetPath(),
                                       std::vector<std::string>{ "*.json" }, "Particle system JSON files", false);
                if (result) {
                    const auto filename = result.value();
                    const auto guid = GUID::create();
                    const auto& particleSystem =
                            mAssetDatabase.loadParticleSystem(filename, guid, mAssetDatabase.texture(mTextureGUID),
                                                              mAssetDatabase.shaderProgramMutable(mShaderProgramGUID));
                    const auto hasBeenLoaded = mAssetDatabase.hasBeenLoaded(guid);
                    if (!hasBeenLoaded) {
                        openErrorPopup = true;
                    } else {
                        if (mParticleEmitterEntity != invalidEntity) {
                            mRegistry.destroyEntity(mParticleEmitterEntity);
                        }
                        mParticleEmitterEntity =
                                mRegistry.createEntity(TransformComponent{}, RootComponent{},
                                                       ParticleEmitterComponent{ .particleSystem{ &particleSystem },
                                                                                 .lastSpawnTime{ mTime.elapsed } });
                    }
                }
            }
            if (ImGui::MenuItem("Exit")) {
                mAppContext.application.quit();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    if (openErrorPopup) {
        ImGui::OpenPopup("Error");
    }
    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Failed to load particle system!");
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::EndPopup();
    }
}
