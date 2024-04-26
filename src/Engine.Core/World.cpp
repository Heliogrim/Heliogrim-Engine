#include "World.hpp"

#include <Engine.Scene/SceneBase.hpp>

using namespace hg::engine::core;
using namespace hg;

World::World(mref<uptr<scene::SceneBase>> scene_) :
	_scene(std::move(scene_)) {}

World::~World() = default;

nmpt<engine::scene::SceneBase> World::getScene() const noexcept {
	return _scene.get();
}
