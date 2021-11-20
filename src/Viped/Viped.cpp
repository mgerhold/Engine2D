//
// Created by coder2k on 09.10.2021.
//

#include "Viped.hpp"
#include "Config.hpp"
#include "FileUtils/FileUtils.hpp"
#include <ImGuiUtils/Bezier.hpp>
#include <AssetDatabase.hpp>
#include <Component.hpp>
#include <Platform.hpp>
#include <tinyfiledialogs/tinyfiledialogs.h>
#include <spdlog/spdlog.h>
#include <range/v3/all.hpp>
#include <gsl/gsl>
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <filesystem>
#include <iostream>
#include <string>
#include <cinttypes>

using namespace c2k;

void Viped::setup() noexcept {
    mTextureGUID = GUID::create();      // initialize to a yet unknown GUID
    mShaderProgramGUID = GUID::create();// same
}

void Viped::update() noexcept {
    if (mParticleEmitterEntity == invalidEntity) {
        return;
    }
    auto& emitterTransform = mRegistry.componentMutable<TransformComponent>(mParticleEmitterEntity).value();
    const auto elapsed = mTime.elapsed * mParticleEmitterMovementSpeedFactor;
    switch (mParticleEmitterMovementPattern) {
        case ParticleEmitterMovementPattern::None:
            emitterTransform.position = glm::vec3{ 0.0f };
            break;
        case ParticleEmitterMovementPattern::HorizontalBounce:
            emitterTransform.position =
                    glm::vec3{ gsl::narrow_cast<float>(std::sin(elapsed)) * mParticleEmitterMovementRadius, 0.0f,
                               0.0f };
            break;
        case ParticleEmitterMovementPattern::VerticalBounce:
            emitterTransform.position =
                    glm::vec3{ 0.0f, gsl::narrow_cast<float>(std::sin(elapsed)) * mParticleEmitterMovementRadius,
                               0.0f };
            break;
        case ParticleEmitterMovementPattern::Circle:
            emitterTransform.position = glm::vec3{ gsl::narrow_cast<float>(std::cos(elapsed)),
                                                   gsl::narrow_cast<float>(std::sin(elapsed)), 0.0f } *
                                        mParticleEmitterMovementRadius;
            break;
    }
}

void Viped::renderImGui() noexcept {
    renderMainMenu();
    renderTextureList();
    renderParticleSystemList();
    renderStatsWindow();
    renderInspectorWindow();
    renderParticleEmitterWindow();
    ImGui::ShowDemoWindow();
}

tl::expected<std::monostate, std::string> Viped::onNewProjectClicked() noexcept {
    const auto result = openFileDialog("Select Asset List", c2k::AssetDatabase::assetPath(),
                                       std::vector<std::string>{ "*.json" }, "Asset list JSON files", false);
    if (result) {
        const auto& filename = result.value();
        const auto searchResult = FileUtils::findFileInParent(filename.parent_path(), engineConfigurationFileFilename);
        if (searchResult) {
            const auto& configFilename = searchResult.value();
            spdlog::info("Found configuration file: {}", configFilename.string());
            const auto deserializationResult = JSON::fromFileAs<Config>(configFilename);
            if (!deserializationResult) {
                return tl::unexpected(
                        fmt::format("Unable to deserialize configuration file: {}", deserializationResult.error()));
            }
            auto config = deserializationResult.value();
            if (!config.assetDirectory) {
                spdlog::info("No asset directory specified, defaulting to 'assets'");
                config.assetDirectory = std::string{ "assets" };
            }
            AssetDatabase::setWorkingDirectory(configFilename.parent_path());
            spdlog::info("Setting working directory to {}", AssetDatabase::workingDirectory().string());
            ;
            AssetDatabase::setAssetPath(AssetDatabase::workingDirectory() / config.assetDirectory.value());
            spdlog::info("Setting asset path to {}", AssetDatabase::assetPath().string());
        } else {
            spdlog::error("Unable to find configuration file: {}", searchResult.error());
            return tl::unexpected(searchResult.error());
        }
        auto assetList = AssetList::fromFile(filename);
        if (!assetList) {
            return tl::unexpected(fmt::format("Failed to read asset list: {}", assetList.error()));
        }
        mAssetList = std::move(assetList.value());
    }
    return std::monostate{};
}

void Viped::renderMainMenu() noexcept {
    tl::expected<std::monostate, std::string> result{ std::monostate{} };
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Project...")) {
                result = onNewProjectClicked();
            }
            if (ImGui::MenuItem("Exit")) {
                mAppContext.application.quit();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    static std::string errorMessage = "";
    if (!result) {
        ImGui::OpenPopup("Error");
        errorMessage = result.error();
    }
    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s", errorMessage.c_str());
        constexpr auto buttonWidth = 120;
        const auto windowWidth = ImGui::GetWindowSize().x;
        ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
        if (ImGui::Button("OK", ImVec2(buttonWidth, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::EndPopup();
    }
}

void Viped::renderTextureList() noexcept {
    ImGui::Begin("Available Textures", nullptr);
    if (ImGui::BeginTable("textureTable", 2, tableFlags)) {
        ImGui::TableSetupColumn("Textures");
        ImGui::TableSetupColumn("Selection", ImGuiTableColumnFlags_WidthFixed, 105.0f);
        ImGui::TableHeadersRow();
        if (mAssetList.assetDescriptions().textures) {
            for (const auto& textureDescription : mAssetList.assetDescriptions().textures.value()) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", textureDescription.filename.string().c_str());
                ImGui::TableNextColumn();
                ImGui::PushID(textureDescription.guid.string().c_str());
                if (ImGui::Button("Select", ImVec2(100, 0))) {
                    spdlog::info("Button clicked!");
                    if (mParticleSystem) {
                        mParticleSystemDescription.texture = textureDescription.guid;
                        changeTexture(textureDescription);
                    }
                }
                ImGui::PopID();
            }
        }
        ImGui::EndTable();
    }
    ImGui::End();
}

void Viped::renderParticleSystemList() noexcept {
    ImGui::Begin("Particle Systems", nullptr);
    if (mParticleSystem) {
        ImGui::Text("Currently open:");
        ImGui::Text("%s", mParticleSystemDescription.filename.string().c_str());
        if (ImGui::Button("Save (Overwrite)")) {
            saveParticleSystem();
        }
        ImGui::SameLine();
        if (ImGui::Button("Close")) {
            closeParticleSystem();
        }
    } else if (ImGui::BeginTable("particleSystemsTable", 2, tableFlags)) {
        ImGui::TableSetupColumn("Particle System");
        ImGui::TableSetupColumn("Open", ImGuiTableColumnFlags_WidthFixed, 105.0f);
        ImGui::TableHeadersRow();
        if (mAssetList.assetDescriptions().particleSystems) {
            for (const auto& particleSystemDescription : mAssetList.assetDescriptions().particleSystems.value()) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", particleSystemDescription.filename.string().c_str());
                ImGui::TableNextColumn();
                if (ImGui::Button("Open", ImVec2(100, 0))) {
                    spdlog::info("Refreshing particle system");
                    refreshParticleSystem(particleSystemDescription);
                }
            }
        }
        ImGui::EndTable();
    }
    ImGui::End();
}

void Viped::refreshParticleSystem(
        const c2k::AssetDescriptions::ParticleSystemDescription& particleSystemDescription) noexcept {
    if (!mAssetList.assetDescriptions().textures) {
        spdlog::warn("Warning: No textures in asset list");
    } else {
        const auto& textureDescriptions = mAssetList.assetDescriptions().textures.value();
        const auto it =
                ranges::find_if(textureDescriptions, [&particleSystemDescription](const auto& textureDescription) {
                    return particleSystemDescription.texture == textureDescription.guid;
                });
        if (it != textureDescriptions.end()) {
            if (mAssetDatabase.hasBeenLoaded(mTextureGUID)) {
                spdlog::info("Unloading previously loaded texture");
                mAssetDatabase.unload(mTextureGUID);
            }
            mTextureGUID = it->guid;
            mAssetDatabase.loadTexture(AssetDatabase::assetPath() / it->filename, it->guid);
        } else {
            spdlog::warn("Warning: Texture with given GUID {} could not be found in asset list",
                         particleSystemDescription.texture.string());
        }
    }

    // TODO: load shader program

    if (mParticleSystem) {
        mAssetDatabase.unload(mParticleSystemDescription.guid);
    }
    mParticleSystemDescription = particleSystemDescription;

    auto& particleSystem = mAssetDatabase.loadParticleSystem(
            AssetDatabase::assetPath() / particleSystemDescription.filename, particleSystemDescription.guid,
            mAssetDatabase.texture(mTextureGUID), mAssetDatabase.shaderProgramMutable(mShaderProgramGUID));
    if (mParticleEmitterEntity != invalidEntity) {
        mRegistry.destroyEntity(mParticleEmitterEntity);
    }
    mRegistry.template destroyEntitiesWithComponents<ParticleComponent>();
    mParticleEmitterEntity = mRegistry.createEntity(
            TransformComponent{}, RootComponent{},
            ParticleEmitterComponent{ .particleSystem{ particleSystem }, .lastSpawnTime{ mTime.elapsed } });
    mParticleSystem = &mRegistry.componentMutable<ParticleEmitterComponent>(mParticleEmitterEntity)->particleSystem;
}

void Viped::renderStatsWindow() const noexcept {
    ImGui::Begin("Stats");
    ImGui::Text("Render Batches: %zu", mRenderer.stats().numBatches);
    ImGui::Text("Number of Quads: %zu", mRenderer.stats().numTriangles / 2);
    ImGui::Separator();
    ImGui::Text("Number of Entities: %zu", mRegistry.numEntities());
    ImGui::Indent();
    ImGui::Text("Alive: %zu", mRegistry.numEntitiesAlive());
    ImGui::Text("Dead: %zu", mRegistry.numEntitiesDead());
    ImGui::Unindent();
    ImGui::Separator();
    ImGui::Text("Alive Entities:");
    ImGui::Indent();
    for (const auto entity : mRegistry.entitiesAlive()) {
        std::string components;
        if (mRegistry.hasComponent<ParticleEmitterComponent>(entity)) {
            components += "emitter, ";
        }
        if (mRegistry.hasComponent<DynamicSpriteComponent>(entity)) {
            components += "sprite, ";
        }
        if (mRegistry.hasComponent<CameraComponent>(entity)) {
            components += "camera, ";
        }
        if (mRegistry.hasComponent<ParticleComponent>(entity)) {
            components += "particle, ";
        }
        if (mRegistry.hasComponent<RootComponent>(entity)) {
            components += "root, ";
        }
        if (mRegistry.hasComponent<RelationshipComponent>(entity)) {
            components +=
                    fmt::format("relationship {}, ", mRegistry.component<RelationshipComponent>(entity).value().parent);
        }
        ImGui::Text("%u|%u (%s)", Registry::getIdentifierBitsFromEntity(entity),
                    Registry::getGenerationBitsFromEntity(entity), components.c_str());
    }
    ImGui::Unindent();
    ImGui::Text("Dead Entities:");
    ImGui::Indent();
    for (const auto entity : mRegistry.entitiesDead()) {
        ImGui::Text("%u|%u", Registry::getIdentifierBitsFromEntity(entity),
                    Registry::getGenerationBitsFromEntity(entity));
    }
    ImGui::Unindent();
    ImGui::End();
}

void Viped::renderInspectorWindow() noexcept {
    ImGui::Begin("Inspector");
    if (mParticleSystem) {
        ImGui::Text("Duration: %0.2f s / %0.2f s", mParticleSystem->currentDuration, mParticleSystem->duration);
        if (ImGui::CollapsingHeader("Duration & Looping")) {
            ImGui::PushID("Duration & Looping");
            dragDouble("Duration", &mParticleSystem->duration, 0.05, 0.0, std::numeric_limits<double>::max());
            ImGui::Checkbox("Looping", &mParticleSystem->looping);
            ImGui::PopID();
        }
        mStartLifeTimeSelector(mParticleSystem->startLifetime);
        mStartDelaySelector(mParticleSystem->startDelay);
        mStartSpeedSelector(mParticleSystem->startSpeed);
        mStartSizeSelector(mParticleSystem->startSize);
        if (ImGui::CollapsingHeader("Simulation Space")) {
            ImGui::Checkbox("Simulate in World Space", &mParticleSystem->simulateInWorldSpace);
        }
    }
    ImGui::End();
}

void Viped::changeTexture(const c2k::AssetDescriptions::TextureDescription& textureDescription) noexcept {
    if (mParticleSystem == nullptr) {
        return;
    }
    const Texture* const texture =
            (mAssetDatabase.hasBeenLoaded(textureDescription.guid)
                     ? &mAssetDatabase.texture(textureDescription.guid)
                     : &mAssetDatabase.loadTexture(AssetDatabase::assetPath() / textureDescription.filename,
                                                   textureDescription.guid));
    mParticleSystem->sprite.texture = texture;
}

void Viped::closeParticleSystem() noexcept {
    if (!mParticleSystem) {
        spdlog::error("Cannot close particle system since none is currently opened");
        return;
    }
    mRegistry.destroyEntity(mParticleEmitterEntity);
    mRegistry.destroyEntitiesWithComponents<ParticleComponent>();
    mAssetDatabase.unload(mTextureGUID);
    mAssetDatabase.unload(mShaderProgramGUID);
    mAssetDatabase.unload(mParticleSystem->guid);
    mParticleSystem = nullptr;
    mTextureGUID = GUID{};
    mShaderProgramGUID = GUID{};
    mParticleEmitterEntity = invalidEntity;
    mParticleSystemDescription = AssetDescriptions::ParticleSystemDescription{};
}

void Viped::saveParticleSystem() noexcept {
    if (!mParticleSystem) {
        spdlog::error("Cannot save particle system since none is currently opened");
        return;
    }
    using namespace c2k::ParticleSystemImpl;
    const auto serializableRepresentation = ParticleSystemJSON{ *mParticleSystem };
    const auto json = JSON::Value{ serializableRepresentation };
    const auto success = json.dumpToFile(AssetDatabase::assetPath() / mParticleSystemDescription.filename);
    if (!success) {
        spdlog::error("Unable to write particle system: {}", success.error());
        return;
    }
    // TODO: write texture/shader into asset list
}

void Viped::renderParticleEmitterWindow() noexcept {
    ImGui::Begin("Particle Emitter Settings");
    if (mParticleEmitterEntity != invalidEntity) {
        const char* patternStrings[] = { "None", "Horizontal Bounce", "Vertical Bounce", "Circle" };
        auto selectedPattern = static_cast<int>(mParticleEmitterMovementPattern);
        ImGui::Combo("Movement Pattern", &selectedPattern, patternStrings,
                     gsl::narrow_cast<int>(std::size(patternStrings)));
        mParticleEmitterMovementPattern = static_cast<ParticleEmitterMovementPattern>(selectedPattern);
        ImGui::DragFloat("Movement Radius", &mParticleEmitterMovementRadius, 0.05f, 0.0f,
                         std::numeric_limits<float>::max());
        dragDouble("Movement Speed Factor", &mParticleEmitterMovementSpeedFactor, 0.05, 0.0,
                   std::numeric_limits<double>::max());
    }
    ImGui::End();
}
