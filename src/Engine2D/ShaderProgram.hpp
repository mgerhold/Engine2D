//
// Created by coder2k on 15.05.2021.
//

#pragma once

#include "GUID.hpp"
#include <glad/glad.h>
#pragma warning(push)
#pragma warning(disable: 4201)
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#pragma warning(pop)
#include <unordered_map>

namespace c2k {

    class ShaderProgram final {
    public:
        ShaderProgram() = default;
        ShaderProgram(const ShaderProgram&) = delete;
        ShaderProgram(ShaderProgram&& other) noexcept;

        ShaderProgram& operator=(const ShaderProgram&) = delete;
        ShaderProgram& operator=(ShaderProgram&& other) noexcept;

        ~ShaderProgram();

        [[nodiscard]] bool compile(const std::string& vertexShaderSource,
                                   const std::string& fragmentShaderSource) noexcept;
        static void bind(GLuint shaderName) noexcept;
        void bind() const noexcept;
        static void unbind() noexcept;
        [[nodiscard]] bool hasBeenCompiled() const noexcept {
            return mName != 0U;
        }
        static tl::expected<ShaderProgram, std::string> generateFromFiles(
                const std::filesystem::path& vertexShaderPath,
                const std::filesystem::path& fragmentShaderPath);
        static void setUniform(GLuint shaderName, std::size_t uniformNameHash, const glm::mat4& matrix) noexcept;
        void setUniform(std::size_t uniformNameHash, const glm::mat4& matrix) const noexcept;
        [[nodiscard]] static ShaderProgram defaultProgram() noexcept;

    public:
        GUID guid;

    private:
        void cacheUniformLocations() noexcept;

    private:
        static GLuint sCurrentlyBoundName;
        GLuint mName{ 0U };
        std::unordered_map<std::size_t, GLint> mUniformLocations;

        friend class Renderer;
    };

}// namespace c2k