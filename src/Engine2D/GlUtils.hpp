//
// Created by coder2k on 15.05.2021.
//

#pragma once

#include "glad/glad.h"
#include <cstddef>

namespace c2k::GLUtils {

    bool isIntegralType(GLenum dataType);
    std::size_t getSizeOfGLType(GLenum dataType);

}// namespace c2k::GLUtils
