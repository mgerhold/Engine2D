#include "OpenGLVersion.hpp"
#include "Sandbox.hpp"
#include "WindowSize.hpp"
#include <cstdlib>
#include <memory>

int main() {
    using namespace c2k;

    Sandbox sandbox{ "OpenGL application", WindowSize{ .width{ 800 }, .height{ 600 } },
                     OpenGLVersion{ .major{ 4 }, .minor{ 5 } } };
    sandbox.run();
    return EXIT_SUCCESS;
}
