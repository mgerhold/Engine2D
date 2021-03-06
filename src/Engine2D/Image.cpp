//
// Created by coder2k on 05.06.2021.
//

#include "Image.hpp"

namespace c2k {

    tl::expected<Image, std::string> Image::loadFromFile(const std::filesystem::path& filename,
                                                         int numChannels) noexcept {
        if (!std::filesystem::exists(filename)) {
            return tl::unexpected{ fmt::format("File not found: {}", filename.string()) };
        }
        Image result;
        // TODO: handle unicode filenames under Windows (see stb_image.h, line 181,
        //       and https://en.cppreference.com/w/cpp/filesystem/path/string)
        stbi_set_flip_vertically_on_load(true);
        result.mData = Image::Pointer{ stbi_load(filename.string().c_str(), &result.mWidth, &result.mHeight,
                                                 &result.mNumChannels, numChannels) };
        if (!result.mData) {
            return tl::unexpected{ fmt::format("stb_image failed to load image: {}", stbi_failure_reason()) };
        }
        if (numChannels != 0) {
            result.mNumChannels = numChannels;
        }
        return result;
    }

    int Image::getWidth() const noexcept {
        return mWidth;
    }

    int Image::getHeight() const noexcept {
        return mHeight;
    }

    int Image::getNumChannels() const noexcept {
        return mNumChannels;
    }

    unsigned char* Image::getData() const noexcept {
        return mData.get();
    }

    Image::Image(Image&& other) noexcept {
        using std::swap;
        swap(mWidth, other.mWidth);
        swap(mHeight, other.mHeight);
        swap(mNumChannels, other.mNumChannels);
        swap(mData, other.mData);
    }

    Image& Image::operator=(Image&& other) noexcept {
        using std::swap;
        swap(mWidth, other.mWidth);
        swap(mHeight, other.mHeight);
        swap(mNumChannels, other.mNumChannels);
        swap(mData, other.mData);
        return *this;
    }

    void Image::Deleter::operator()(unsigned char* const data) {
        stbi_image_free(data);
    }

}// namespace c2k