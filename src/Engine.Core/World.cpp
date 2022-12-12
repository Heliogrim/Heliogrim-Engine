#include "World.hpp"

#include <Engine.Scene/Scene.hpp>

using namespace ember::engine::core;
using namespace ember;

World::World(mref<uptr<Scene>> scene_) :
    _scene(_STD move(scene_)) {}

World::~World() = default;

const non_owning_rptr<World::Scene> World::getScene() const noexcept {
    return _scene.get();
}
