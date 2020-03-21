#include "../include/World.hpp"
#include "../include/components.hpp"
#include "../include/CollisionSystem.hpp"
#include "../include/MovementSystem.hpp"
#include "../include/WallSystem.hpp"
#include "../include/GravitySystem.hpp"
#include <vector>
#include <unordered_map>
#include <chrono>

void gravity::World::update() {
    using namespace std::chrono;
    auto deltaTime = duration<double>(high_resolution_clock::now() - _updateTimer).count();
    _updateTimer = high_resolution_clock::now();

    if (_interruptFlag) { deltaTime = 0; _interruptFlag = false; }
    for (auto &system : _systems) { system->update(deltaTime); }
}

void gravity::World::draw() {
    using namespace std::chrono;
    using namespace systems;

    auto deltaTime = duration<double>(high_resolution_clock::now() - _drawTimer).count();
    _drawTimer = high_resolution_clock::now();
    if (_interruptFlag) { deltaTime = 0; _interruptFlag = false; }
    // update local to worlds components
    // cull w/ Circle collider
    // update ltw buffer
    _renderer->update(deltaTime);
}

gravity::World::World() : _bounds{-4000,-4000,4000,4000}, _registry(), _executor(), _renderer(new systems::RenderSystem(*this)) {
    using namespace components;
    using namespace systems;
    using namespace mathsimd;
    static constexpr int n = 12;
    static_assert(n > 1);
    std::vector<entt::entity> entities(n * n);

    _registry.create(entities.begin(), entities.end());
    auto const start = _bounds.min + float2(2500,2500);
    int i = 0;
    for (auto &e : entities) {
        int dx = i % n;
        int dy = i / n;
        auto pos = start + 3000.f/static_cast<float>(n - 1) * float2(static_cast<float>(dx),static_cast<float>(dy));
        float r = _rand.rnd(12.f,30.f);
        _registry.assign<Position>(e, pos);
        _registry.assign<Velocity>(e, _rand.rnd(-124.f,124.f),_rand.rnd(-124.f,124.f));
        _registry.assign<Acceleration>(e, mathsimd::float2::zero());
        _registry.assign<CircleCollider>(e, r);
        _registry.assign<Mass>(e, _rand.rnd(100.f, 800000.f));
        _registry.assign<Restitution>(e, _rand.rnd(0.5f,1.f));
        _registry.assign<LocalToWorld>(e, LocalToWorld::fromPositionAndRadius(pos, r));
        ++i;
    }

    _systems.emplace_back(new MovementSystem(*this));
    _systems.emplace_back(new GravitySystem(*this, _registry.size(), 0.0005f));
    _systems.emplace_back(new CollisionSystem(*this, 80, 80, _registry.size()));
    _systems.emplace_back(new WallSystem(*this));
    _updateTimer = std::chrono::high_resolution_clock::now();
    _drawTimer = std::chrono::high_resolution_clock::now();
}

gravity::World::~World() {
    _systems.clear();
    delete _renderer;
}
