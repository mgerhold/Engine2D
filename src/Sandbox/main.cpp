#include "Sandbox.hpp"
#include <SparseArray.hpp>
#include <cstdlib>
#include <string>

int main() {
    // sparse array test code
    SparseArray<std::string> sparseArray{ 100 };
    sparseArray[42] = "The answer";
    spdlog::info("'{}'", sparseArray[42]);
    spdlog::info("'{}'", sparseArray[10]);

    for (auto& str : sparseArray) {
        str += " - concat";
    }
    for (const auto& str : sparseArray) {
        spdlog::info("'{}'", str);
    }

    // sandbox application
    Sandbox sandbox{ "OpenGL application", WindowSize{ .width{ 800 }, .height{ 600 } },
                     OpenGLVersion{ .major{ 4 }, .minor{ 5 } } };
    if (sandbox.hasError()) {
        return EXIT_FAILURE;
    }
    sandbox.run();
}
