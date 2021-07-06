//
// Created by coder2k on 19.06.2021.
//

#include "Renderer.hpp"
#include "GLDataUsagePattern.hpp"
#include "ScopedTimer.hpp"
#include <gsl/gsl>
#include <spdlog/spdlog.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <execution>

Renderer::Renderer()
    : mVertexBuffer(GLDataUsagePattern::DynamicDraw,
                    maxCommandsPerBatch * 4ULL * sizeof(VertexData),
                    maxCommandsPerBatch * 6ULL * sizeof(IndexData)) {
    mCommandBuffer.resize(maxCommandsPerBatch);
    mVertexData.resize(maxCommandsPerBatch * 4ULL);
    mIndexData.resize(maxCommandsPerBatch * 6ULL);
    mCommandIterator = mCommandBuffer.begin();
    mVertexIterator = mVertexData.begin();
    mIndexIterator = mIndexData.begin();
    mCurrentTextureNames.reserve(std::min(Texture::getTextureUnitCount(), 32));
    spdlog::info("GPU is capable of binding {} textures at a time.", mCurrentTextureNames.capacity());
    mVertexBuffer.setVertexAttributeLayout(
            VertexAttributeDefinition{ 3, GL_FLOAT, false }, VertexAttributeDefinition{ 4, GL_FLOAT, false },
            VertexAttributeDefinition{ 2, GL_FLOAT, false }, VertexAttributeDefinition{ 1, GL_UNSIGNED_INT, false });
}

void Renderer::beginFrame() noexcept {
    mVertexIterator = mVertexData.begin();
    mIndexIterator = mIndexData.begin();
    mRenderStats = RenderStats{};
}

void Renderer::endFrame() noexcept {
    flushCommandBuffer();
    flushVertexAndIndexData();
}

void Renderer::drawQuad(const glm::vec3& translation,
                        float rotationAngle,
                        const glm::vec3& scale,
                        const ShaderProgram& shader,
                        const Texture& texture) noexcept {
    drawQuad(glm::scale(glm::rotate(glm::translate(glm::mat4{ 1.0f }, translation), rotationAngle,
                                    glm::vec3{ 0.0f, 0.0f, 1.0f }),
                        scale),
             shader, texture);
}

template<typename T>
void Renderer::drawQuad(T&& transform, const ShaderProgram& shader, const Texture& texture) noexcept {
    if (mCommandIterator == mCommandBuffer.end()) {
        flushCommandBuffer();
    }
    *mCommandIterator++ = RenderCommand{ .transform = transform,
                                         .color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f },
                                         .shaderName = shader.mName,
                                         .textureName = texture.mName };
}

void Renderer::flushCommandBuffer() noexcept {
    SCOPED_TIMER();
    if (mRenderStats.numBatches == 0ULL) {
        glClear(GL_COLOR_BUFFER_BIT);
    }
    if (mCommandIterator == mCommandBuffer.begin()) {
        return;
    }
    spdlog::info("Flushing {} commands", std::distance(mCommandBuffer.begin(), mCommandIterator));

    {
        SCOPED_TIMER_NAMED("Sorting");
        std::sort(mCommandBuffer.begin(), mCommandIterator, [](const RenderCommand& lhs, const RenderCommand& rhs) {
            // TODO: sort differently for transparent shaders
            return (lhs.shaderName < rhs.shaderName) ||
                   (lhs.shaderName == rhs.shaderName && lhs.textureName < rhs.textureName);
        });
    }
    auto currentStartIt = mCommandBuffer.begin();
    auto currentEndIt = std::upper_bound(
            mCommandBuffer.begin(), mCommandIterator, mCommandBuffer.front(),
            [](const RenderCommand& lhs, const RenderCommand& rhs) { return lhs.shaderName < rhs.shaderName; });

    while (currentStartIt != mCommandIterator) {// one iteration per shader
        mVertexIterator = mVertexData.begin();
        mIndexIterator = mIndexData.begin();
        mCurrentTextureNames.clear();
        ShaderProgram::bind(currentStartIt->shaderName);
        {
            SCOPED_TIMER_NAMED("commands to data");
            std::for_each(currentStartIt, currentEndIt, [&](const RenderCommand& renderCommand) {
                addVertexAndIndexDataFromRenderCommand(renderCommand);
            });
        }

        currentStartIt = currentEndIt;
        if (currentEndIt != mCommandBuffer.end()) {// there's at least one more shader to draw with
            currentEndIt = std::upper_bound(
                    currentEndIt, mCommandIterator, *currentEndIt,
                    [](const RenderCommand& lhs, const RenderCommand& rhs) { return lhs.shaderName < rhs.shaderName; });
        }
        flushVertexAndIndexData();
    }
    mCommandIterator = mCommandBuffer.begin();
}

void Renderer::flushVertexAndIndexData() noexcept {
    if (mVertexIterator == mVertexData.begin()) {
        return;
    }
    // flush all buffers
    {
        SCOPED_TIMER_NAMED("submit data");
        mVertexBuffer.submitVertexData(mVertexData.begin(), mVertexIterator);
        mVertexBuffer.submitIndexData(mIndexData.begin(), mIndexIterator);
    }
    mVertexBuffer.bind();
    for (std::size_t i = 0; i < mCurrentTextureNames.size(); ++i) {
        Texture::bind(mCurrentTextureNames[i], gsl::narrow_cast<GLint>(i));
    }

    spdlog::info("Drawing {} elements ({} tris)", mVertexBuffer.indicesCount(), mVertexBuffer.indicesCount() / 3);
    glDrawElements(GL_TRIANGLES, gsl::narrow_cast<GLsizei>(mVertexBuffer.indicesCount()), GL_UNSIGNED_INT, nullptr);
    mVertexIterator = mVertexData.begin();
    mIndexIterator = mIndexData.begin();
    mCurrentTextureNames.clear();
    mNumTrianglesInCurrentBatch = 0ULL;
    mRenderStats.numBatches += 1ULL;
}

void Renderer::addVertexAndIndexDataFromRenderCommand(const Renderer::RenderCommand& renderCommand) {
    // TODO: use an indirection vector to optimize this as soon as there is a global asset manager
    GLuint textureIndex = 0;
    bool foundTexture = false;

    for (std::size_t i = 0; i < mCurrentTextureNames.size(); ++i) {
        if (mCurrentTextureNames[i] == renderCommand.textureName) {
            textureIndex = gsl::narrow_cast<GLuint>(i);
            foundTexture = true;
            break;
        }
    }

    if ((!foundTexture && mCurrentTextureNames.size() == mCurrentTextureNames.capacity()) ||
        (std::distance(mVertexIterator, mVertexData.end()) < 4)) {
        spdlog::info("iterator: {}, end: {}", std::distance(mVertexData.begin(), mVertexIterator),
                     std::distance(mVertexData.begin(), mVertexData.end()));
        flushVertexAndIndexData();
    }
    if (!foundTexture) {
        textureIndex = static_cast<GLuint>(mCurrentTextureNames.size());
        mCurrentTextureNames.push_back(renderCommand.textureName);
    }

    const auto indexOffset = gsl::narrow_cast<GLuint>(mVertexData.size());
    *mVertexIterator++ = VertexData{ .position = renderCommand.transform * glm::vec4{ -1.0f, -1.0f, 0.0f, 1.0f },
                                     .color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f },
                                     .texCoords = glm::vec2{ 0.0f, 0.0f },
                                     .texIndex = textureIndex };
    *mVertexIterator++ = VertexData{ .position = renderCommand.transform * glm::vec4{ 1.0f, -1.0f, 0.0f, 1.0f },
                                     .color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f },
                                     .texCoords = glm::vec2{ 1.0f, 0.0f },
                                     .texIndex = textureIndex };
    *mVertexIterator++ = VertexData{ .position = renderCommand.transform * glm::vec4{ 1.0f, 1.0f, 0.0f, 1.0f },
                                     .color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f },
                                     .texCoords = glm::vec2{ 1.0f, 1.0f },
                                     .texIndex = textureIndex };
    *mVertexIterator++ = VertexData{ .position = renderCommand.transform * glm::vec4{ -1.0f, 1.0f, 0.0f, 1.0f },
                                     .color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f },
                                     .texCoords = glm::vec2{ 0.0f, 1.0f },
                                     .texIndex = textureIndex };
    *mIndexIterator++ = IndexData{ .i0 = indexOffset + 0, .i1 = indexOffset + 1, .i2 = indexOffset + 2 };
    *mIndexIterator++ = IndexData{ .i0 = indexOffset + 0, .i1 = indexOffset + 2, .i2 = indexOffset + 3 };
    /*mVertexData.emplace_back(renderCommand.transform * glm::vec4{ -1.0f, -1.0f, 0.0f, 1.0f },
                             glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f }, glm::vec2{ 0.0f, 0.0f }, textureIndex);
    mVertexData.emplace_back(renderCommand.transform * glm::vec4{ 1.0f, -1.0f, 0.0f, 1.0f },
                             glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f }, glm::vec2{ 1.0f, 0.0f }, textureIndex);
    mVertexData.emplace_back(renderCommand.transform * glm::vec4{ 1.0f, 1.0f, 0.0f, 1.0f },
                             glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f }, glm::vec2{ 1.0f, 1.0f }, textureIndex);
    mVertexData.emplace_back(renderCommand.transform * glm::vec4{ -1.0f, 1.0f, 0.0f, 1.0f },
                             glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f }, glm::vec2{ 0.0f, 1.0f }, textureIndex);
    mIndexData.emplace_back(indexOffset + 0, indexOffset + 1, indexOffset + 2);
    mIndexData.emplace_back(indexOffset + 0, indexOffset + 2, indexOffset + 3);*/

    mNumTrianglesInCurrentBatch += 2ULL;
    mRenderStats.numVertices += 4ULL;
    mRenderStats.numTriangles += 2ULL;
}
