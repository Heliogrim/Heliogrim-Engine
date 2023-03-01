#include "World.hpp"

#include <Engine.Scene/Scene.hpp>

using namespace hg::engine::core;
using namespace hg;

World::World(mref<uptr<Scene>> scene_) :
    _scene(_STD move(scene_)) {}

World::~World() = default;

const non_owning_rptr<World::Scene> World::getScene() const noexcept {
    return _scene.get();
}
