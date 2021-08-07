#include "Sandbox.hpp"
#include "SparseSet.hpp"
#include "Registry.hpp"
#include <range/v3/all.hpp>
#include <cstdlib>
#include <limits>
#include <string>
#include <tuple>

int main() {
    // sandbox application
    Sandbox sandbox{ "OpenGL application", WindowSize{ .width{ 800 }, .height{ 600 } },
                     OpenGLVersion{ .major{ 4 }, .minor{ 5 } } };
    if (sandbox.hasError()) {
        return EXIT_FAILURE;
    }
    sandbox.run();
    return EXIT_SUCCESS;
}
