#include "Sandbox.hpp"
#include "SparseSet.hpp"
#include "Registry.hpp"
#include <cstdlib>
#include <string>
#include <range/v3/all.hpp>
#include <tuple>

using Entity = uint32_t;

struct Position {
    float x, y;
};

struct Velocity {
    float x, y;
};

int main() {
    // sandbox application
    Sandbox sandbox{ "OpenGL application", WindowSize{ .width{ 800 }, .height{ 600 } },
                     OpenGLVersion{ .major{ 4 }, .minor{ 5 } } };
    if (sandbox.hasError()) {
        return EXIT_FAILURE;
    }
    sandbox.run();
}
