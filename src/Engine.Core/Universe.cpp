#include "Universe.hpp"

#include <Engine.Scene/SceneBase.hpp>

using namespace hg::engine::core;
using namespace hg;

engine::core::Universe::Universe(mref<uptr<scene::SceneBase>> scene_) :
	_scene(std::move(scene_)),
	_levels() {}

engine::core::Universe::~Universe() = default;

nmpt<engine::scene::SceneBase> engine::core::Universe::getScene() const noexcept {
	return _scene.get();
}
