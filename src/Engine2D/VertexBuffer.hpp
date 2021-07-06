//
// Created by coder2k on 22.05.2021.
//

#pragma once

#include "VertexAttributeDefinition.hpp"
#include "GLDataUsagePattern.hpp"
#include "GlUtils.hpp"
#include <glad/glad.h>
#include <gsl/gsl_util>
#include <concepts>
#include <vector>
#include <algorithm>
#include <span>
#include <spdlog/spdlog.h>

class VertexBuffer {
public:
    VertexBuffer(GLDataUsagePattern usagePattern,
                 GLsizeiptr initialVertexBufferCapacityInBytes = 0LL,
                 GLsizeiptr initialIndexBufferCapacityInBytes = 0LL) noexcept;
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer(VertexBuffer&& other) noexcept;
    ~VertexBuffer();

    VertexBuffer& operator=(const VertexBuffer&) = delete;
    VertexBuffer& operator=(VertexBuffer&& other) noexcept;

    void bind() const noexcept;
    static void unbind() noexcept;

    [[nodiscard]] std::size_t indicesCount() const noexcept {
        return mNumIndices;
    }
    void setVertexAttributeLayout(std::convertible_to<VertexAttributeDefinition> auto... args) const;

    template<typename VertexData>
    void submitVertexData(std::span<VertexData> data) noexcept {
        bindVertexArrayObject();
        const GLsizeiptr size = data.size() * sizeof(typename decltype(data)::value_type);
        if (size > mCurrentVertexBufferSize) {
            glBufferData(GL_ARRAY_BUFFER, size, data.data(), static_cast<GLenum>(mDataUsagePattern));
            mCurrentVertexBufferSize = size;
        } else {
            glBufferSubData(GL_ARRAY_BUFFER, 0, size, data.data());
        }
    }

    template<typename VertexData>
    void submitVertexData(VertexData&& data) noexcept {
        submitVertexData(std::span{ std::forward<VertexData>(data) });
    }

    template<typename Iterator>
    void submitVertexData(Iterator begin, Iterator end) noexcept {
        submitVertexData(std::span{ begin, end });
    }

    template<typename IndexData>
    void submitIndexData(std::span<IndexData> data) noexcept {
        bindElementBufferObject();
        const GLsizeiptr size = data.size() * sizeof(typename decltype(data)::value_type);
        if (size > mCurrentIndexBufferSize) {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data.data(), static_cast<GLenum>(mDataUsagePattern));
            mCurrentIndexBufferSize = size;
        } else {
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size, data.data());
        }
        // TODO: handle the possibility of varying data type for OpenGL indices
        static_assert(sizeof(IndexData::i0) == 4);
        mNumIndices = data.size() * sizeof(typename decltype(data)::value_type) / sizeof(IndexData::i0);
    }

    template<typename IndexData>
    void submitIndexData(IndexData&& data) noexcept {
        submitIndexData(std::span{ std::forward<IndexData>(data) });
    }

    template<typename Iterator>
    void submitIndexData(Iterator begin, Iterator end) noexcept {
        submitIndexData(std::span{ begin, end });
    }

private:
    void bindVertexArrayObject() const noexcept;
    void bindVertexBufferObject() const noexcept;
    void bindElementBufferObject() const noexcept;
    static void unbindVertexArrayObject() noexcept;
    static void unbindVertexBufferObject() noexcept;
    static void unbindElementBufferObject() noexcept;

private:
    static inline GLuint sCurrentlyBoundVertexArrayObjectName{ 0U };
    static inline GLuint sCurrentlyBoundVertexBufferObjectName{ 0U };
    static inline GLuint sCurrentlyBoundElementBufferObjectName{ 0U };
    GLuint mVertexArrayObjectName{ 0U };
    GLuint mVertexBufferObjectName{ 0U };
    GLuint mElementBufferObjectName{ 0U };
    std::size_t mNumIndices{ 0U };
    GLsizeiptr mCurrentVertexBufferSize{ 0LL };
    GLsizeiptr mCurrentIndexBufferSize{ 0LL };
    GLDataUsagePattern mDataUsagePattern;
};

void VertexBuffer::setVertexAttributeLayout(std::convertible_to<VertexAttributeDefinition> auto... args) const {
    bind();
    const auto values = { args... };
    GLuint location{ 0U };
    std::uintptr_t offset{ 0U };
    GLsizei stride{ 0 };
    // calculate stride
    std::for_each(values.begin(), values.end(), [&stride](const VertexAttributeDefinition& definition) {
        stride += gsl::narrow_cast<GLsizei>(GLUtils::getSizeOfGLType(definition.type) * definition.count);
    });

    // set vertex attributes
    std::for_each(values.begin(), values.end(),
                  [&location, &offset, stride](const VertexAttributeDefinition& definition) {
                      if (GLUtils::isIntegralType(definition.type)) {
                          glVertexAttribIPointer(location, definition.count, definition.type, stride, (void*) offset);
                      } else {
                          glVertexAttribPointer(location, definition.count, definition.type, definition.normalized,
                                                stride, (void*) offset);
                      }
                      glEnableVertexAttribArray(location);
                      spdlog::info(
                              "Enabled vertex attribute {} (count {}, type {}, normalized {}, stride {}, "
                              "offset {})",
                              location, definition.count, definition.type, definition.normalized, stride, offset);
                      ++location;
                      offset += GLUtils::getSizeOfGLType(definition.type) * definition.count;
                  });
}
