cmake_minimum_required(VERSION 3.19)
project(Engine2D)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    # enable code analysis
    set_property(GLOBAL PROPERTY VS_GLOBAL_EnableCppCoreCheck true)
    set_property(GLOBAL PROPERTY VS_GLOBAL_CodeAnalysisRuleSet CppCoreCheckRules.ruleset)
    set_property(GLOBAL PROPERTY VS_GLOBAL_RunCodeAnalysis true)

    # remove /W3 from defaults
    string(REGEX REPLACE "/W3" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(REGEX REPLACE "-W3" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
endif ()

set(TARGET_LIST
        Engine2D
        Sandbox
        )

add_library(Engine2D STATIC
        src/Engine2D/Application.hpp
        src/Engine2D/strong_type/strong_type.hpp
        src/Engine2D/WindowSize.hpp
        src/Engine2D/OpenGLVersion.hpp
        src/Engine2D/GLDataUsagePattern.hpp
        src/Engine2D/ShaderProgram.cpp
        src/Engine2D/ShaderProgram.hpp
        src/Engine2D/VertexAttributeDefinition.hpp
        src/Engine2D/GlUtils.cpp
        src/Engine2D/GlUtils.hpp
        src/Engine2D/VertexBuffer.cpp
        src/Engine2D/VertexBuffer.hpp
        src/Engine2D/expected/expected.hpp
        src/Engine2D/hash/hash.hpp
        src/Engine2D/hash/hash.cpp
        src/Engine2D/StbImageInclude.cpp
        src/Engine2D/Image.cpp
        src/Engine2D/Image.hpp
        src/Engine2D/Texture.cpp
        src/Engine2D/Texture.hpp
        src/Engine2D/Renderer.cpp
        src/Engine2D/Renderer.hpp
        src/Engine2D/ScopedTimer.cpp
        src/Engine2D/ScopedTimer.hpp
        src/Engine2D/Input.cpp
        src/Engine2D/Input.hpp
        src/Engine2D/SparseSet.hpp
        src/Engine2D/ComponentHolder.hpp
        src/Engine2D/TypeIdentifier.hpp
        src/Engine2D/Registry.hpp
        src/Engine2D/Entity.hpp
        src/Engine2D/optional/optional.hpp
        src/Engine2D/Component.hpp
        src/Engine2D/System.hpp
        src/Engine2D/AssetDatabase.cpp
        src/Engine2D/AssetDatabase.hpp
        src/Engine2D/GUID.hpp
        src/Engine2D/Color.hpp
        src/Engine2D/Window.cpp
        src/Engine2D/Window.hpp
        src/Engine2D/Time.hpp
        src/Engine2D/AssetList.cpp
        src/Engine2D/AssetList.hpp
        src/Engine2D/FileUtils.cpp
        src/Engine2D/FileUtils.hpp
        src/Engine2D/PredefinedSystems.hpp
        src/Engine2D/Random.cpp
        src/Engine2D/Random.hpp
        src/Engine2D/pch.hpp
        src/Engine2D/TypeErasedVector.cpp
        src/Engine2D/TypeErasedVector.hpp
        src/Engine2D/ComponentHolderPairIterator.hpp
        src/Engine2D/SpriteSheet.hpp
        src/Engine2D/SpriteSheet.cpp
        src/Engine2D/JSONUtils.hpp
        src/Engine2D/Rect.hpp
        src/Engine2D/GUID.cpp
        src/Engine2D/ApplicationContext.hpp
        src/Engine2D/PredefinedSystems.cpp
        src/Engine2D/Script.cpp
        src/Engine2D/Script.hpp
        src/Engine2D/ScriptUtils.cpp
        src/Engine2D/ScriptUtils.hpp
        src/Engine2D/ApplicationContext.cpp
        src/Engine2D/Registry.cpp
        src/Engine2D/SparseSet.cpp)
add_executable(Sandbox src/Sandbox/main.cpp src/Sandbox/Sandbox.hpp src/Sandbox/Sandbox.cpp)
target_include_directories(Sandbox PUBLIC ${PROJECT_SOURCE_DIR}/src/Engine2D)
add_dependencies(Sandbox Engine2D)

# enable profiling if desired
if (ENABLE_PROFILING)
    add_definitions(-DENABLE_PROFILING=1)
endif ()

add_definitions(-DBOOST_UUID_FORCE_AUTO_LINK)

foreach (target ${TARGET_LIST})
    # set warning levels
    if (MSVC)
        target_compile_options(${target} PUBLIC /W4 /permissive-)
    else ()
        target_compile_options(${target} PUBLIC -Wall -Wextra -pedantic -Wconversion -pthread -ltbb)
    endif ()

    # define DEBUG_BUILD
    target_compile_definitions(${target} PUBLIC "$<$<CONFIG:DEBUG>:DEBUG_BUILD>")

    # static runtime library
    set_property(TARGET ${target} PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    # set binary filenames
    set_target_properties(${target} PROPERTIES OUTPUT_NAME_DEBUG ${target}-debug)
    set_target_properties(${target} PROPERTIES OUTPUT_NAME_RELWITHDEBINFO ${target}-relwithdebinfo)
    set_target_properties(${target} PROPERTIES OUTPUT_NAME_RELEASE ${target}-release)
    set_target_properties(${target} PROPERTIES OUTPUT_NAME_MINSIZEREL ${target}-minsizerel)
endforeach ()

# precompiled headers
target_precompile_headers(Engine2D PRIVATE "src/Engine2D/pch.hpp")
target_precompile_headers(Sandbox PRIVATE "src/Engine2D/pch.hpp")

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(Engine2D PRIVATE glfw)

find_package(glad CONFIG REQUIRED)
target_link_libraries(Engine2D PRIVATE glad::glad)
target_link_libraries(Sandbox PRIVATE glad::glad)

find_package(Microsoft.GSL CONFIG REQUIRED)
target_link_libraries(Engine2D PRIVATE Microsoft.GSL::GSL)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(Engine2D PRIVATE spdlog::spdlog spdlog::spdlog_header_only)

find_package(glm CONFIG REQUIRED)
target_link_libraries(Engine2D PRIVATE glm::glm)

find_package(range-v3 CONFIG REQUIRED)
target_link_libraries(Engine2D PRIVATE range-v3 range-v3-meta range-v3::meta range-v3-concepts)

find_path(STB_INCLUDE_DIRS "stb.h")
target_include_directories(Engine2D PRIVATE ${STB_INCLUDE_DIRS})

find_package(Boost REQUIRED COMPONENTS random)
target_link_libraries(Engine2D PRIVATE ${Boost_LIBRARIES})
target_include_directories(Engine2D PRIVATE ${Boost_INCLUDE_DIRS})

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(Engine2D PRIVATE nlohmann_json nlohmann_json::nlohmann_json)

include(FindLua)
find_package(Lua REQUIRED)
target_link_libraries(Engine2D PRIVATE ${LUA_LIBRARIES})
target_include_directories(Engine2D PRIVATE ${LUA_INCLUDE_DIR})

find_package(sol2 CONFIG REQUIRED)
target_link_libraries(Engine2D PRIVATE sol2::sol2)

target_link_libraries(Sandbox PRIVATE spdlog::spdlog spdlog::spdlog_header_only)
target_link_libraries(Sandbox PRIVATE Engine2D)

enable_testing()
add_subdirectory(tests)
