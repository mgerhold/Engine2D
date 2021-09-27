//
// Created by coder2k on 21.08.2021.
//

#pragma once

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <spdlog/spdlog.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <nlohmann/json.hpp>
#include <tl/expected.hpp>
#include <tl/optional.hpp>
#pragma warning(push)
#pragma warning(disable : 4201)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#pragma warning(pop)
#pragma warning(push)
#pragma warning(disable : 4702)
#include <range/v3/all.hpp>
#pragma warning(pop)
#include <stb_image.h>
#include <gsl/gsl>
#include <gsl/gsl_util>
#include <sol/sol.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <concepts>
#include <execution>
#include <filesystem>
#include <functional>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <new>
#include <numeric>
#include <optional>
#include <ratio>
#include <random>
#include <source_location>
#include <span>
#include <streambuf>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <cctype>