#include "../include/World.hpp"
#include "../include/components.hpp"
#include "../include/CollisionSystem.hpp"
#include "../include/MovementSystem.hpp"
#include "../include/WallSystem.hpp"
#include "../include/RenderSystem.hpp"
#include "../include/GravitySystem.hpp"
#include "../include/CameraControlSystem.hpp"
#include <vector>
#include <unordered_map>
#include <chrono>

gravity::World::World() : _bounds{-16000,-16000,16000,16000}, _registry(), _executor() {
    using namespace components;
    using namespace systems;
    using namespace mathsimd;
    static constexpr int n = 25;
    static_assert(n > 1);
    std::vector<entt::entity> entities(n * n);
    auto max_r = 150.f;
    auto min_r = 10.f;
    _registry.create(entities.begin(), entities.end());
    auto size = _bounds.size() * 0.5f;
    auto wm = size.magnitude();
    int i = 0;
    for (auto &e : entities) {
        auto pos = (_rand.rnd_vec<mathsimd::float2>() - 0.5f) * size;
        float r = (i < 300) ? _rand.rnd(min_r,  20.f) : _rand.rnd(20.f,  max_r);
        auto m = pos.magnitude();
        auto tmp = pos.normalized();

        float t = atan2(tmp.y(), tmp.x());
        _registry.assign<Position>(e, pos);
        _registry.assign<Velocity>(e, 0.01f * wm * exp(-m/wm)  * float2(-sin(t), cos(t)));
        _registry.assign<Acceleration>(e, mathsimd::float2::zero());
        _registry.assign<CircleCollider>(e, r);
        _registry.assign<Mass>(e, (i < 300) ? _rand.rnd(100.f, 800.f) : _rand.rnd(10000.f, 80000.f));
        _registry.assign<Restitution>(e, _rand.rnd(0.1f,.4f));
        _registry.assign<LocalToWorld>(e, LocalToWorld::fromPositionAndRadius(pos, r));
        ++i;
    }
    auto e = _registry.create();
    _registry.assign<BlackHole>(e);
    _registry.assign<Position>(e, float2::zero());
    _registry.assign<Velocity>(e, float2::zero());
    _registry.assign<Acceleration>(e, mathsimd::float2::zero());
    _registry.assign<CircleCollider>(e, 10);
    _registry.assign<Mass>(e, 800000000);
    _registry.assign<Restitution>(e, 0);
    _registry.assign<LocalToWorld>(e, LocalToWorld::fromPositionAndRadius(float2::zero(), 10));

    auto limit = mathsimd::float2(4,4);
    auto s = _bounds.size() / (limit * min_r);
    _simulation.emplace_back(new MovementSystem(*this));
    _simulation.emplace_back(new GravitySystem(*this, _registry.size(), 0.0005f));
    _simulation.emplace_back(new CollisionSystem(*this, static_cast<int>(s.x()), static_cast<int>(s.y()), _registry.size()));
    _simulation.emplace_back(new WallSystem(*this));

    _presentation.emplace_back(new CameraControlSystem(*this));
    _presentation.emplace_back(new RenderSystem(*this));

    _updateTimer = std::chrono::high_resolution_clock::now();
    _predrawTimer = std::chrono::high_resolution_clock::now();

    assert(dynamic_cast<RenderSystem*>(_presentation.back().get()));
}

float gravity::World::update(float delta_dt) {
    using namespace std::chrono;

    auto deltaTime = std::clamp(duration<double>(high_resolution_clock::now() - _updateTimer).count() - delta_dt, 0.0, 1.0);
    _updateTimer = high_resolution_clock::now();

    for (auto &system : _simulation) { system->update(deltaTime); }

    return std::clamp(duration<double>(high_resolution_clock::now() - _updateTimer).count(), 0.0, 1.0);;
}

float gravity::World::predraw(float delta_dt) {
    using namespace std::chrono;

    auto deltaTime = std::clamp(duration<double>(high_resolution_clock::now() - _predrawTimer).count() - delta_dt, 0.0, 1.0);
    _predrawTimer = high_resolution_clock::now();

    for (auto &system : _presentation) system->update(deltaTime);

    return std::clamp(duration<double>(high_resolution_clock::now() - _predrawTimer).count(), 0.0, 1.0);
}

gravity::World::~World() {
    _simulation.clear();
}
