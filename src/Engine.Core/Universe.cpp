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

engine::core::Universe::~Universe() {
	for (auto& level : _levels) {
		uncommitLevel(level.get());
	}
	_rootLevel.reset();
	_levels.clear();
}

nmpt<engine::scene::SceneBase> engine::core::Universe::getScene() const noexcept {
	return _scene.get();
}

void engine::core::Universe::commitLevel(nmpt<core::Level> level_) {
	const auto ctx = _scene->registerContext();
	for (auto actor : level_->getActors()) {
		actor->registerComponents(*this, *ctx.get());
	}
}

void engine::core::Universe::uncommitLevel(nmpt<core::Level> level_) {
	auto& ctx = _scene->registerContext().load<>();
	for (auto actor : level_->getActors()) {
		// TODO: unregister actors and components from scene
		actor->unregisterComponents(ctx);
	}
}

void engine::core::Universe::addLevel(mref<Arci<core::Level>> level_) {
	_levels.emplace(clone(level_));
	commitLevel(level_.get());
}

std::span<const Arci<Level>> engine::core::Universe::getLevels() const noexcept {
	return _levels.values();
}

ref<const Arci<Level>> engine::core::Universe::getRootLevel() const noexcept {
	return *_rootLevel;
}

void engine::core::Universe::removeLevel(cref<Arci<core::Level>> level_) {
	::hg::assertrt(level_ != _rootLevel);

	const auto keepAlive = clone(level_);
	_levels.erase(level_);
	uncommitLevel(keepAlive.get());
}