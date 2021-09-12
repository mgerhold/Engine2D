//
// Created by coder2k on 05.06.2021.
//

#include "Texture.hpp"

namespace c2k {

    tl::expected<Texture, std::string> Texture::create(const Image& image) noexcept {
        GLint colorComponentFormat;
        const int numChannels = image.getNumChannels();
        switch (numChannels) {
            case 3:
                colorComponentFormat = GL_RGB;
                break;
            case 4:
                colorComponentFormat = GL_RGBA;
                break;
            default:
                return tl::unexpected{ fmt::format("Unsupported number of channels: {}", numChannels) };
        }

        Texture result;
        glGenTextures(1, &result.mName);
        result.bind();
        glTexImage2D(GL_TEXTURE_2D, 0, colorComponentFormat, image.getWidth(), image.getHeight(), 0,
                     colorComponentFormat, GL_UNSIGNED_BYTE, image.getData());
        glGenerateMipmap(GL_TEXTURE_2D);
        result.mWidth = image.getWidth();
        result.mHeight = image.getHeight();
        result.mNumChannels = image.getNumChannels();
        result.setFiltering(Filtering::Linear);
        result.setWrap(false);
        return result;
    }

    tl::expected<Texture, std::string> Texture::createFromMemory(int width,
                                                                 int height,
                                                                 int numChannels,
                                                                 unsigned char* data) noexcept {
        GLint colorComponentFormat;
        switch (numChannels) {
            case 3:
                colorComponentFormat = GL_RGB;
                break;
            case 4:
                colorComponentFormat = GL_RGBA;
                break;
            default:
                return tl::unexpected{ fmt::format("Unsupported number of channels: {}", numChannels) };
        }

        Texture result;
        glGenTextures(1, &result.mName);
        result.bind();
        glTexImage2D(GL_TEXTURE_2D, 0, colorComponentFormat, width, height, 0, colorComponentFormat, GL_UNSIGNED_BYTE,
                     data);
        glGenerateMipmap(GL_TEXTURE_2D);
        result.mWidth = width;
        result.mHeight = height;
        result.mNumChannels = numChannels;
        result.setFiltering(Filtering::Linear);
        result.setWrap(true);
        return result;
    }

    tl::expected<Texture, std::string> Texture::createFromFillColor(int width,
                                                                    int height,
                                                                    int numChannels,
                                                                    Color fillColor) noexcept {
        using ranges::views::ints;
        auto bufferLength{ static_cast<std::size_t>(width * height * numChannels) };
        auto buffer = std::make_unique<unsigned char[]>(bufferLength);
        for (auto i : ints(0, width * height)) {
            buffer[i * numChannels + 0] = fillColor.r;
            buffer[i * numChannels + 1] = fillColor.g;
            buffer[i * numChannels + 2] = fillColor.b;
            if (numChannels == 4) {
                buffer[i * numChannels + 3] = fillColor.a;
            }
        }
        return createFromMemory(width, height, numChannels, buffer.get());
    }


    void Texture::bind(GLint textureUnit) const noexcept {
        /*if (textureUnit < 0 || textureUnit >= getTextureUnitCount()) {
        spdlog::error("Cannot bind texture since {} is no valid texture unit.", textureUnit);
        return;
    }
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, mName);*/
        bind(mName, textureUnit);
    }

    void Texture::unbind(GLint textureUnit) noexcept {
        if (textureUnit < 0 || textureUnit >= getTextureUnitCount()) {
            spdlog::error("Cannot unbind texture since {} is no valid texture unit.", textureUnit);
            return;
        }
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    GLint Texture::getTextureUnitCount() noexcept {
        if (sTextureUnitCount != 0) {
            return sTextureUnitCount;
        }
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &sTextureUnitCount);
        return sTextureUnitCount;
    }

    void Texture::setFiltering(Texture::Filtering filtering) const noexcept {
        // TODO: Remove unnecessary binds
        bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering == Filtering::Linear ? GL_LINEAR : GL_NEAREST);
    }

    void Texture::setWrap(bool enabled) const noexcept {
        // TODO: Remove unnecessary binds
        bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, enabled ? GL_REPEAT : GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, enabled ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    }

    Texture::Texture(Texture&& other) noexcept {
        using std::swap;
        swap(mName, other.mName);
        swap(mWidth, other.mWidth);
        swap(mHeight, other.mHeight);
        swap(mNumChannels, other.mNumChannels);
        swap(guid, other.guid);
    }

    Texture::~Texture() {
        glDeleteTextures(1, &mName);
    }

    Texture& Texture::operator=(Texture&& other) noexcept {
        using std::swap;
        swap(mName, other.mName);
        swap(mWidth, other.mWidth);
        swap(mHeight, other.mHeight);
        swap(mNumChannels, other.mNumChannels);
        swap(guid, other.guid);
        return *this;
    }

    void Texture::bind(GLuint textureName, GLint textureUnit) noexcept {
        if (textureUnit < 0 || textureUnit >= getTextureUnitCount()) {
            spdlog::error("Cannot bind texture since {} is no valid texture unit.", textureUnit);
            return;
        }
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, textureName);
    }

}// namespace c2k