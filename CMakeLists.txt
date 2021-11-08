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
        Viped
        Pong
        FlappyBird
        )

add_library(Engine2D STATIC
        src/Engine2D/Application.hpp
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
        src/Engine2D/Component.hpp
        src/Engine2D/AssetDatabase.cpp
        src/Engine2D/AssetDatabase.hpp
        src/Engine2D/GUID.hpp
        src/Engine2D/Color.hpp
        src/Engine2D/Window.cpp
        src/Engine2D/Window.hpp
        src/Engine2D/Time.hpp
        src/Engine2D/AssetList.cpp
        src/Engine2D/AssetList.hpp
        src/Engine2D/FileUtils/FileUtils.cpp
        src/Engine2D/FileUtils/FileUtils.hpp
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
        src/Engine2D/Script.cpp
        src/Engine2D/Script.hpp
        src/Engine2D/ScriptUtils/ScriptUtils.cpp
        src/Engine2D/ScriptUtils/ScriptUtils.hpp
        src/Engine2D/ApplicationContext.cpp
        src/Engine2D/SparseSet.cpp
        src/Engine2D/BufferedScriptCommand.hpp
        src/Engine2D/BufferedScriptCommands.cpp
        src/Engine2D/Hash/Hash.hpp
        src/Engine2D/Hash/Hash.cpp
        src/Engine2D/Application.cpp
        src/Engine2D/ParticleSystem.hpp
        src/Engine2D/ParticleSystem.cpp
        src/Engine2D/MathUtils/MathUtils.cpp
        src/Engine2D/MathUtils/MathUtils.hpp
        src/Engine2D/Sprite.cpp
        src/Engine2D/Sprite.hpp
        src/Engine2D/JSON/JSON.cpp
        src/Engine2D/JSON/JSON.hpp
        src/Engine2D/JSON/MacroDefinitions.hpp
        src/Engine2D/JSON/Parsers.cpp
        src/Engine2D/JSON/Parsers.hpp
        src/Engine2D/Animation.hpp src/Engine2D/Platform.cpp src/Engine2D/Platform.hpp src/Engine2D/ImGuiUtils/Bezier.cpp src/Engine2D/ImGuiUtils/Bezier.hpp)
target_include_directories(Engine2D PUBLIC ${PROJECT_SOURCE_DIR}/src/Engine2D)

add_executable(Sandbox src/Sandbox/main.cpp src/Sandbox/Sandbox.hpp src/Sandbox/Sandbox.cpp)
add_executable(Viped src/Viped/Main.cpp src/Viped/Viped.hpp src/Viped/Viped.cpp src/Viped/VipedProject.hpp src/Viped/FourWayVariantSelector.hpp src/Viped/Utility.cpp src/Viped/Utility.hpp)
add_executable(Pong src/Pong/Main.cpp src/Pong/Pong.cpp src/Pong/Pong.hpp)
add_executable(FlappyBird src/FlappyBird/Main.cpp)

# enable profiling if desired
if (ENABLE_PROFILING)
    add_definitions(-DENABLE_PROFILING=1)
endif ()

add_definitions(-DBOOST_UUID_FORCE_AUTO_LINK)

foreach (target ${TARGET_LIST})
    # set warning levels
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        message("MSVC build")
        target_compile_options(${target} PUBLIC /W4 /permissive- /bigobj)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message("GCC build")
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

    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        message("Enabling LTO for target ${target}")
        set_property(TARGET ${target} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else ()
        message("Not enabling LTO for target ${target} (not a release build)")
    endif ()
endforeach ()

# precompiled headers
target_precompile_headers(Engine2D PRIVATE "src/Engine2D/pch.hpp")

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(Engine2D PRIVATE glfw)

find_package(glad CONFIG REQUIRED)
target_link_libraries(Engine2D PRIVATE glad::glad)

find_package(Microsoft.GSL CONFIG REQUIRED)
target_link_libraries(Engine2D PRIVATE Microsoft.GSL::GSL)

find_package(tl-expected CONFIG REQUIRED)
target_link_libraries(Engine2D PRIVATE tl::expected)

find_package(tl-optional CONFIG REQUIRED)
target_link_libraries(Engine2D PRIVATE tl::optional)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(Engine2D PUBLIC spdlog::spdlog spdlog::spdlog_header_only)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(Engine2D PRIVATE nlohmann_json nlohmann_json::nlohmann_json)

find_package(glm CONFIG REQUIRED)
target_link_libraries(Engine2D PRIVATE glm::glm)

find_package(range-v3 CONFIG REQUIRED)
target_link_libraries(Engine2D PRIVATE range-v3 range-v3-meta range-v3::meta range-v3-concepts)

find_path(STB_INCLUDE_DIRS "stb.h")
target_include_directories(Engine2D PRIVATE ${STB_INCLUDE_DIRS})

find_package(Boost REQUIRED COMPONENTS random)
target_link_libraries(Engine2D PUBLIC ${Boost_LIBRARIES})
target_include_directories(Engine2D PUBLIC ${Boost_INCLUDE_DIRS})

include(FindLua)
find_package(Lua REQUIRED)
target_link_libraries(Engine2D PRIVATE ${LUA_LIBRARIES})
target_include_directories(Engine2D PRIVATE ${LUA_INCLUDE_DIR})

find_package(sol2 CONFIG REQUIRED)
target_link_libraries(Engine2D PRIVATE sol2::sol2)

find_package(magic_enum CONFIG REQUIRED)
target_link_libraries(Engine2D PRIVATE magic_enum::magic_enum)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(Engine2D PUBLIC imgui::imgui)

target_link_libraries(Sandbox PRIVATE spdlog::spdlog spdlog::spdlog_header_only)

target_link_libraries(Sandbox PRIVATE Engine2D)
target_link_libraries(Viped PRIVATE Engine2D)
target_link_libraries(Pong PRIVATE Engine2D)
target_link_libraries(FlappyBird PRIVATE Engine2D)

find_package(tinyfiledialogs CONFIG REQUIRED)
target_link_libraries(Viped PRIVATE tinyfiledialogs::tinyfiledialogs)

enable_testing()
add_subdirectory(tests)
