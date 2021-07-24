#include "Sandbox.hpp"
#include "SparseSet.hpp"
#include "ComponentHolder.hpp"
#include <cstdlib>
#include <string>
#include <range/v3/all.hpp>

using Entity = uint32_t;

struct Position {
    float x, y;
};

struct Velocity {
    float x, y;
};

int main() {
    ComponentHolder<Entity> registry{ 1000 };
    for (Entity i = 10; i < 20; ++i) {
        registry.addComponent<Position>(i, Position{ 2.0f, 3.0f });
        if (i % 2 == 0) {
            registry.addComponent<Velocity>(i, Velocity{ 2.0f, 3.0f });
        }
    }
    for (auto entity : registry.getComponents<Position, Velocity>()) {
        spdlog::info(entity);
    }
    spdlog::info("=============");
    for (auto entity : registry.getComponents<Position>()) {
        spdlog::info(entity);
    }

    // sandbox application
    Sandbox sandbox{ "OpenGL application", WindowSize{ .width{ 800 }, .height{ 600 } },
                     OpenGLVersion{ .major{ 4 }, .minor{ 5 } } };
    if (sandbox.hasError()) {
        return EXIT_FAILURE;
    }
    sandbox.run();
}
