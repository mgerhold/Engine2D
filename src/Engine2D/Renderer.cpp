//
// Created by coder2k on 19.06.2021.
//

#include "Renderer.hpp"
#include "Component.hpp"
#include "GLDataUsagePattern.hpp"
#include "ScopedTimer.hpp"
#include "hash/hash.hpp"

#include "pch.hpp"

namespace c2k {

    Renderer::Renderer(const Window& window)
        : mVertexBuffer(GLDataUsagePattern::StreamDraw,
                        maxCommandsPerBatch * 4ULL * sizeof(VertexData),
                        maxCommandsPerBatch * 6ULL * sizeof(IndexData)),
          mWindow{ window } {
        mCommandBuffer.resize(maxCommandsPerBatch);
        mVertexData.resize(maxCommandsPerBatch * 4ULL);
        mIndexData.resize(maxCommandsPerBatch * 6ULL);
        mCommandIterator = mCommandBuffer.begin();
        mVertexIterator = mVertexData.begin();
        mIndexIterator = mIndexData.begin();
        mCurrentTextureNames.reserve(std::min(Texture::getTextureUnitCount(), 32));
        spdlog::info("GPU is capable of binding {} textures at a time.", mCurrentTextureNames.capacity());
        mVertexBuffer.setVertexAttributeLayout(VertexAttributeDefinition{ 3, GL_FLOAT, false },
                                               VertexAttributeDefinition{ 4, GL_FLOAT, false },
                                               VertexAttributeDefinition{ 2, GL_FLOAT, false },
                                               VertexAttributeDefinition{ 1, GL_UNSIGNED_INT, false });
    }

    void Renderer::beginFrame(const glm::mat4& viewMatrix) noexcept {
        mVertexIterator = mVertexData.begin();
        mIndexIterator = mIndexData.begin();
        mRenderStats = RenderStats{};
        mCurrentViewProjectionMatrix = Camera::projectionMatrix(mWindow.framebufferSize()) * viewMatrix;
    }

    void Renderer::endFrame() noexcept {
        flushCommandBuffer();
        flushVertexAndIndexData();
    }

    void Renderer::drawQuad(const glm::vec3& translation,
                            float rotationAngle,
                            const glm::vec2& scale,
                            ShaderProgram& shader,
                            const Texture& texture,
                            const Rect& textureRect,
                            const Color& color) noexcept {
        drawQuad(glm::scale(glm::rotate(glm::translate(glm::mat4{ 1.0f }, translation), rotationAngle,
                                        glm::vec3{ 0.0f, 0.0f, 1.0f }),
                            glm::vec3{ scale.x, scale.y, 1.0f }),
                 shader, texture, textureRect, color);
    }

    template<typename T>
    void Renderer::drawQuad(T&& transform,
                            ShaderProgram& shader,
                            const Texture& texture,
                            const Rect& textureRect,
                            const Color& color) noexcept {
        if (mCommandIterator == mCommandBuffer.end()) {
            flushCommandBuffer();
        }
        *mCommandIterator++ = RenderCommand{ .transform{ transform },
                                             .textureRect{ textureRect },
                                             .color{ color },
                                             .shader{ &shader },
                                             .texture{ &texture } };
    }

    void Renderer::flushCommandBuffer() noexcept {
        SCOPED_TIMER();
        if (mCommandIterator == mCommandBuffer.begin()) {
            return;
        }
        {
            SCOPED_TIMER_NAMED("Sorting");
            std::sort(mCommandBuffer.begin(), mCommandIterator, [](const RenderCommand& lhs, const RenderCommand& rhs) {
                // TODO: sort differently for transparent shaders
                return (lhs.shader->mName < rhs.shader->mName) ||
                       (lhs.shader->mName == rhs.shader->mName && lhs.texture->mName < rhs.texture->mName);
            });
        }
        auto currentStartIt = mCommandBuffer.begin();
        auto currentEndIt = std::upper_bound(mCommandBuffer.begin(), mCommandIterator, mCommandBuffer.front(),
                                             [](const RenderCommand& lhs, const RenderCommand& rhs) {
                                                 return lhs.shader->mName < rhs.shader->mName;
                                             });

        while (currentStartIt != mCommandIterator) {// one iteration per shader
            mVertexIterator = mVertexData.begin();
            mIndexIterator = mIndexData.begin();
            mCurrentTextureNames.clear();
            currentStartIt->shader->bind();
            currentStartIt->shader->setUniform(Hash::staticHashString("projectionMatrix"),
                                               mCurrentViewProjectionMatrix);
            {
                SCOPED_TIMER_NAMED("commands to data");
                std::for_each(currentStartIt, currentEndIt, [&](const RenderCommand& renderCommand) {
                    addVertexAndIndexDataFromRenderCommand(renderCommand);
                });
            }

            currentStartIt = currentEndIt;
            if (currentEndIt != mCommandBuffer.end()) {// there's at least one more shader to draw with
                currentEndIt = std::upper_bound(currentEndIt, mCommandIterator, *currentEndIt,
                                                [](const RenderCommand& lhs, const RenderCommand& rhs) {
                                                    return lhs.shader->mName < rhs.shader->mName;
                                                });
            }
            flushVertexAndIndexData();
        }
        mCommandIterator = mCommandBuffer.begin();
    }

    void Renderer::flushVertexAndIndexData() noexcept {
        if (mVertexIterator == mVertexData.begin()) {
            return;
        }
        mVertexBuffer.bind();
        // flush all buffers
        {
            SCOPED_TIMER_NAMED("submit data");
            mVertexBuffer.submitVertexData(mVertexData.begin(), mVertexIterator);
            mVertexBuffer.submitIndexData(mIndexData.begin(), mIndexIterator);
        }
        for (std::size_t i = 0; i < mCurrentTextureNames.size(); ++i) {
            Texture::bind(mCurrentTextureNames[i], gsl::narrow_cast<GLint>(i));
        }
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
            if (mCurrentTextureNames[i] == renderCommand.texture->mName) {
                textureIndex = gsl::narrow_cast<GLuint>(i);
                foundTexture = true;
                break;
            }
        }

        if ((!foundTexture && mCurrentTextureNames.size() == mCurrentTextureNames.capacity()) ||
            (mVertexData.end() - mVertexIterator) < 4) {
            flushVertexAndIndexData();
        }
        if (!foundTexture) {
            textureIndex = static_cast<GLuint>(mCurrentTextureNames.size());
            mCurrentTextureNames.push_back(renderCommand.texture->mName);
        }

        const auto indexOffset = gsl::narrow_cast<GLuint>(mVertexIterator - mVertexData.begin());
        constexpr std::array<glm::vec4, 4> positions{ glm::vec4{ -1.0f, -1.0f, 0.0f, 1.0f },
                                                      glm::vec4{ 1.0f, -1.0f, 0.0f, 1.0f },
                                                      glm::vec4{ 1.0f, 1.0f, 0.0f, 1.0f },
                                                      glm::vec4{ -1.0f, 1.0f, 0.0f, 1.0f } };
        const std::array<glm::vec2, 4> texCoords{
            glm::vec2{ renderCommand.textureRect.left, renderCommand.textureRect.bottom },
            glm::vec2{ renderCommand.textureRect.right, renderCommand.textureRect.bottom },
            glm::vec2{ renderCommand.textureRect.right, renderCommand.textureRect.top },
            glm::vec2{ renderCommand.textureRect.left, renderCommand.textureRect.top }
        };
        const auto color = renderCommand.color.normalized(); /* TODO: change members of Color struct to float
                                                                  to avoid normalization */
        for (std::size_t i = 0; i < 4; ++i) {
            mVertexIterator->position = renderCommand.transform * positions[i];
            mVertexIterator->color = color;
            mVertexIterator->texCoords = texCoords[i];
            mVertexIterator->texIndex = textureIndex;
            ++mVertexIterator;
        }
        for (GLuint i = 1; i <= 2; ++i) {
            mIndexIterator->i0 = indexOffset;
            mIndexIterator->i1 = indexOffset + i;
            mIndexIterator->i2 = indexOffset + i + 1;
            ++mIndexIterator;
        }
        mNumTrianglesInCurrentBatch += 2ULL;
        mRenderStats.numVertices += 4ULL;
        mRenderStats.numTriangles += 2ULL;
    }

    void Renderer::clear(bool colorBuffer, bool depthBuffer) noexcept {
        const auto flags{ gsl::narrow_cast<GLbitfield>(GL_COLOR_BUFFER_BIT * colorBuffer) |
                          (GL_DEPTH_BUFFER_BIT * depthBuffer) };
        assert(flags && "At least one of the flags must be set.");
        glClear(flags);
    }

    void Renderer::setClearColor(const Color& color) noexcept {
        const auto normalized = color.normalized();
        glClearColor(normalized.r, normalized.g, normalized.b, normalized.a);
    }

}// namespace c2k