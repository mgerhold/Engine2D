//
// Created by coder2k on 26.10.2021.
//

#include "Pong.hpp"

using namespace c2k;

void Pong::setup() noexcept {
    const auto scriptGUID = GUID::fromString("903fd1c2-a25d-4e29-a740-5427c046ee4e");
    mAssetDatabase.loadFromList(mAssetDatabase.assetPath() / "assets.json");
    mRegistry.createEntity(ScriptComponent{ &mAssetDatabase.scriptMutable(scriptGUID) });
}

void Pong::update() noexcept { }
