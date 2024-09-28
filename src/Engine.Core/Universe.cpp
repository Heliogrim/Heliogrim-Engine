#include "Universe.hpp"

#include <Engine.Level/Level.hpp>
#include <Engine.Scene/SceneBase.hpp>

using namespace hg::engine::core;
using namespace hg;

engine::core::Universe::Universe(mref<uptr<scene::SceneBase>> scene_, mref<DenseSet<Arci<core::Level>>> levels_) :
	_scene(std::move(scene_)),
	_levels(std::move(levels_)) {
	/**/
	for (const auto& level : _levels) {
		if (level->isRootLike()) {
			_rootLevel = Some(level);
			break;
		}
	}
	::hg::assertrt(_rootLevel.has_value());
}

engine::core::Universe::~Universe() = default;

nmpt<engine::scene::SceneBase> engine::core::Universe::getScene() const noexcept {
	return _scene.get();
}

void engine::core::Universe::addLevel(mref<Arci<core::Level>> level_) {
	_levels.emplace(std::move(level_));
}

std::span<const Arci<Level>> engine::core::Universe::getLevels() const noexcept {
	return _levels.values();
}

cref<Arci<Level>> engine::core::Universe::getRootLevel() const noexcept {
	return *_rootLevel;
}

void engine::core::Universe::removeLevel(cref<Arci<core::Level>> level_) {
	_levels.erase(level_);
}
