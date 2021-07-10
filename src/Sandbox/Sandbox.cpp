//
// Created by coder2k on 15.05.2021.
//

#include "Sandbox.hpp"
#include "Image.hpp"
#include "Texture.hpp"
#include "hash/hash.hpp"
#include "ScopedTimer.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <filesystem>

void Sandbox::setup() noexcept {
#ifdef DEBUG_BUILD
    spdlog::info("This is the debug build");
#else
    spdlog::info("This is the release build");
#endif
    auto imagePath = std::filesystem::current_path() / "assets" / "images";
    if (!exists(imagePath)) {
        spdlog::error("Could not find path to image files: {}", imagePath.string());
    } else {
        for (const auto& directoryEntry : std::filesystem::directory_iterator(imagePath)) {
            auto expectedTexture = Image::LoadFromFile(directoryEntry).and_then(Texture::Create);
            if (expectedTexture) {
                mTextures.push_back(std::move(expectedTexture.value()));
                spdlog::info("Loaded texture: {}", directoryEntry.path().string());
            } else {
                spdlog::warn("Could not load image: {} (Error: {})", directoryEntry.path().string(),
                             expectedTexture.error());
            }
        }
    }

    setupShaders();
    glClearColor(73.f / 255.f, 54.f / 255.f, 87.f / 255.f, 1.f);
}

void Sandbox::update() noexcept {
    processInput();
    render();
}

void Sandbox::processInput() noexcept {
    if (mInput.keyPressed(Key::A)) {// pressed since last frame
        spdlog::info("A pressed");
    }
    if (mInput.keyDown(Key::A)) {// is currently held down?
        spdlog::info("A is currently held down");
    }
    if (mInput.keyReleased(Key::A)) {// released since last frame
        spdlog::info("A released");
    }
    if (mInput.mousePressed(MouseButton::Button0)) {
        spdlog::info("Left mouse button pressed");
    }
    if (mInput.mousePressed(MouseButton::Button1)) {
        spdlog::info("Right mouse button pressed");
    }
    if (mInput.mouseDown(MouseButton::Button0)) {
        const auto mousePosition = mInput.mousePosition();
        spdlog::info("Mouse position: ({},{})", mousePosition.x, mousePosition.y);
    }

    if (glfwGetKey(getGLFWWindowPointer(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(getGLFWWindowPointer(), true);
    }
}

void Sandbox::render() noexcept {
    SCOPED_TIMER();
    const auto framebufferSize = getFramebufferSize();
    const auto projectionMatrix = glm::ortho<float>(gsl::narrow_cast<float>(-framebufferSize.width / 2),
                                                    gsl::narrow_cast<float>(framebufferSize.width / 2),
                                                    gsl::narrow_cast<float>(-framebufferSize.height / 2),
                                                    gsl::narrow_cast<float>(framebufferSize.height / 2), -2.0f, 2.0f);
    for (auto& shaderProgram : mShaderPrograms) {
        shaderProgram.setUniform(Hash::staticHashString("projectionMatrix"), projectionMatrix);
    }

    mRenderer.beginFrame();
    const auto offset = glm::vec3{ -gsl::narrow_cast<float>(getFramebufferSize().width) / 2.0f + 20.0f,
                                   -gsl::narrow_cast<float>(getFramebufferSize().height) / 2.0f + 20.0f, 0.0f };
    constexpr int dimension = 500;
    for (int x = 0; x < dimension; ++x) {
        for (int y = 0; y < dimension; ++y) {
            mRenderer.drawQuad(offset + glm::vec3{ static_cast<float>(x) * 40.0f, static_cast<float>(y) * 40.0f, 0.0f },
                               0.0f, glm::vec3{ 20.0f }, mShaderPrograms[y % mShaderPrograms.size()],
                               mTextures[x % mTextures.size()]);
        }
    }
    const auto mousePosition = mInput.mousePosition();
    mRenderer.drawQuad(
            glm::vec3{ mousePosition.x, mousePosition.y, mInput.mouseDown(MouseButton::Button0) ? -0.5f : 0.5f }, 0.0f,
            glm::vec3{ 100.0f }, mShaderPrograms.front(), mTextures[mTextures.size() - 2]);
    mRenderer.endFrame();
    const RenderStats& stats = mRenderer.stats();
    //spdlog::info("Stats: {} tris, {} vertices ({} batches)", stats.numTriangles, stats.numVertices, stats.numBatches);
}

void Sandbox::setupShaders() noexcept {
    auto expectedShaderProgram =
            ShaderProgram::generateFromFiles(std::filesystem::current_path() / "assets" / "shaders" / "default.vert",
                                             std::filesystem::current_path() / "assets" / "shaders" / "default.frag");
    if (!expectedShaderProgram) {
        spdlog::error("Failed to generate shader program from files: {}", expectedShaderProgram.error());
        return;
    }
    mShaderPrograms.push_back(std::move(expectedShaderProgram.value()));

    expectedShaderProgram =
            ShaderProgram::generateFromFiles(std::filesystem::current_path() / "assets" / "shaders" / "default.vert",
                                             std::filesystem::current_path() / "assets" / "shaders" / "debug.frag");
    if (!expectedShaderProgram) {
        spdlog::error("Failed to generate shader program from files: {}", expectedShaderProgram.error());
        return;
    }
    mShaderPrograms.push_back(std::move(expectedShaderProgram.value()));
}
