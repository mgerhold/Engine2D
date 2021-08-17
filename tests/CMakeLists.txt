add_executable(Tests SparseSet.test.cpp Main.test.cpp Registry.test.cpp GUID.test.cpp)

find_package(GTest CONFIG REQUIRED)
target_link_libraries(Tests PRIVATE GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)
target_link_libraries(Tests PRIVATE Engine2D)
target_include_directories(Tests PUBLIC ${PROJECT_SOURCE_DIR}/src/Engine2D)

# static runtime library
set_property(TARGET Tests PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_test(NAME unit COMMAND Tests)