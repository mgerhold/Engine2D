#include "Sandbox.hpp"
#include "SparseSet.hpp"
#include "ComponentHolder.hpp"
#include <cstdlib>
#include <string>

using Entity = uint32_t;

struct Position {
    float x, y;
};

int main() {
    ComponentHolder<Entity> components{ 1000 };
    auto& positions = components.getMutable<Position>();

    constexpr Entity entity = 100;
    //SparseSet<Position, uint32_t> positions{ 1000, 100 };
    assert(!positions.hasComponent(entity));
    positions.addComponent(entity, Position{ .x = 1.0f, .y = 2.0f });
    assert(positions.hasComponent(100));
    const auto& position = positions.getComponent(entity);
    spdlog::info("({}, {})", position.x, position.y);
    positions.getComponentMutable(entity).x += 4.0f;
    spdlog::info("({}, {})", position.x, position.y);
    for (int i = 0; i < 10; ++i) {
        positions.addComponent(entity + 10 + i, Position{ .x = 42.0f + i, .y = 42.0f });
    }
    positions.deleteComponent(entity + 10 + 3);
    positions.deleteComponent(entity + 10 + 5);
    assert(positions.hasComponent(entity + 10 + 0));
    assert(positions.hasComponent(entity + 10 + 1));
    assert(positions.hasComponent(entity + 10 + 2));
    assert(!positions.hasComponent(entity + 10 + 3));
    assert(positions.hasComponent(entity + 10 + 4));
    assert(!positions.hasComponent(entity + 10 + 5));
    assert(positions.hasComponent(entity + 10 + 6));
    assert(positions.hasComponent(entity + 10 + 7));
    assert(positions.hasComponent(entity + 10 + 8));
    assert(positions.hasComponent(entity + 10 + 9));

    assert(positions.componentCount() == 9);

    for (const auto& [ent, pos] : positions.zipView()) {
        spdlog::info("Entity {} has position.x == {}", ent, pos.x);
    }

    positions.forEachComponent([] (Position& pos) {
        pos.x += 100.0f;
    });
    positions.forEachPair([] (auto ent, Position& pos) {
        pos.x += 10.0f;
        spdlog::info("Entity {} has position.x == {}", ent, pos.x);
    });

    // sandbox application
    /*Sandbox sandbox{ "OpenGL application", WindowSize{ .width{ 800 }, .height{ 600 } },
                     OpenGLVersion{ .major{ 4 }, .minor{ 5 } } };
    if (sandbox.hasError()) {
        return EXIT_FAILURE;
    }
    sandbox.run();*/
}
