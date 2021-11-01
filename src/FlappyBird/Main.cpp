//
// Created by coder2k on 27.10.2021.
//

#include <Application.hpp>
#include <cstdlib>

using namespace c2k;

class FlappyBird : public Application {
    void setup() noexcept override {
        const auto scriptGUID = GUID::fromString("1b0b54bf-d36a-4067-bd60-cc69900bb9bc");
        mAssetDatabase.loadFromList(AssetDatabase::assetPath() / "assets.json");
        mRegistry.createEntity(ScriptComponent{ &mAssetDatabase.scriptMutable(scriptGUID) });
        mRenderer.setClearColor(Color{ 113.0f / 255.0f, 197.0f / 255.0f, 207.0f / 255.0f, 1.0f });
    }

    void update() noexcept override { }
};


int main() {
    FlappyBird game;
    game.run();
    return EXIT_SUCCESS;
}