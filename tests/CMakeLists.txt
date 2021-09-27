add_executable(Tests SparseSet.test.cpp Main.test.cpp Registry.test.cpp GUID.test.cpp TypeErasedVector.test.cpp ComponentHolder.test.cpp JSON.test.cpp)

find_package(GTest CONFIG REQUIRED)
target_link_libraries(Tests PRIVATE GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)
target_link_libraries(Tests PRIVATE Engine2D)
target_include_directories(Tests PUBLIC ${PROJECT_SOURCE_DIR}/src/Engine2D)

# static runtime library
set_property(TARGET Tests PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_precompile_headers(Tests PRIVATE "../src/Engine2D/pch.hpp")

add_test(NAME unit COMMAND Tests)