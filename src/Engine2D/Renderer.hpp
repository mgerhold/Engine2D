//
// Created by coder2k on 19.06.2021.
//

#pragma once

#include "VertexBuffer.hpp"
#include "ShaderProgram.hpp"
#include "Texture.hpp"
#include "Color.hpp"
#include "Window.hpp"
#include "pch.hpp"

namespace c2k {

    struct RenderStats {
        std::uint64_t numBatches{ 0ULL };
        std::uint64_t numTriangles{ 0ULL };
        std::uint64_t numVertices{ 0ULL };
    };

    class Renderer final {
    public:
        struct VertexData {
            glm::vec3 position;
            glm::vec4 color;
            glm::vec2 texCoords;
            GLuint texIndex;
        };
        static_assert(alignof(VertexData) == 4);
        static_assert(sizeof(VertexData[2]) == 2 * sizeof(VertexData));
        static_assert(sizeof(VertexData) == 10 * sizeof(GLfloat));

        struct IndexData {
            GLuint i0, i1, i2;
        };
        static_assert(alignof(IndexData) == 4);
        static_assert(sizeof(IndexData) == 3 * sizeof(GLuint));
        static_assert(sizeof(IndexData[2]) == 2 * sizeof(IndexData));

    public:
        Renderer(const Window& window);

        void beginFrame(const glm::mat4& viewMatrix) noexcept;
        void endFrame() noexcept;
        void drawQuad(const glm::vec3& translation,
                      float rotationAngle,
                      const glm::vec2& scale,
                      ShaderProgram& shader,
                      const Texture& texture,
                      const Color& color = Color::white()) noexcept;
        template<typename T = glm::mat4>
        void drawQuad(T&& transform,
                      ShaderProgram& shader,
                      const Texture& texture,
                      const Color& color = Color::white()) noexcept;
        [[nodiscard]] const RenderStats& stats() const {
            return mRenderStats;
        }
        static void clear(bool colorBuffer, bool depthBuffer) noexcept;
        static void setClearColor(const Color& color) noexcept;

    private:
        struct RenderCommand {
            glm::mat4 transform;
            Color color;
            ShaderProgram* shader;
            const Texture* texture;
        };

    private:
        void flushCommandBuffer() noexcept;
        void flushVertexAndIndexData() noexcept;
        void addVertexAndIndexDataFromRenderCommand(const RenderCommand& renderCommand);

    private:
        static constexpr std::size_t maxCommandsPerBatch = 20'000;
        std::uint64_t mNumTrianglesInCurrentBatch = 0ULL;
        std::vector<RenderCommand> mCommandBuffer;
        std::vector<VertexData> mVertexData;
        std::vector<IndexData> mIndexData;
        decltype(mCommandBuffer)::iterator mCommandIterator;
        decltype(mVertexData)::iterator mVertexIterator;
        decltype(mIndexData)::iterator mIndexIterator;
        VertexBuffer mVertexBuffer;
        RenderStats mRenderStats;
        std::vector<GLuint> mCurrentTextureNames;
        GLuint mCurrentShaderProgramName{ 0U };
        glm::mat4 mCurrentViewProjectionMatrix{ 0.0f };
        const Window& mWindow;
    };

}// namespace c2k