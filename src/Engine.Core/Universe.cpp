#include "Universe.hpp"

#include <Engine.Scene/SceneBase.hpp>

using namespace hg::engine::core;
using namespace hg;

Universe::Universe(mref<uptr<scene::SceneBase>> scene_) :
	_scene(std::move(scene_)) {}

Universe::~Universe() = default;

nmpt<engine::scene::SceneBase> Universe::getScene() const noexcept {
	return _scene.get();
}
