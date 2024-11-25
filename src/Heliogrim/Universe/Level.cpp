#include "Level.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Level/Level.hpp>

#include "Universe.hpp"
#include "Engine.Common/Move.hpp"
#include "Engine.Common/Concurrent/Promise.hpp"
#include "Engine.Core/Universe.hpp"
#include "Heliogrim/Asset/Universe/LevelAsset.hpp"

using namespace hg;

Level::Level() noexcept = default;

Level::Level(mref<Arci<hg::engine::core::Level>> internal_) :
	_internal(std::move(internal_)) {}

Level::Level(cref<Level> other_) noexcept :
	_internal(other_._internal) {}

Level::Level(mref<Level> other_) noexcept :
	_internal(std::move(other_._internal)) {}

Level::~Level() noexcept = default;

ref<Level> Level::operator=(mref<Level>) noexcept = default;

bool Level::valid() const noexcept {
	return _internal != nullptr;
}

cref<decltype(Level::_internal)> Level::unwrap() const noexcept {
	return _internal;
}

Level::Level(mref<Level> other_) noexcept {
	::hg::todo_panic();
}

Level::~Level() noexcept {
	::hg::todo_panic();
}

bool Level::addActor(ptr<Actor> actor_) {
Future<Level> hg::CreateLevel() noexcept {
	auto p = concurrent::promise<Level> { [] { return Level { engine::core::make_root_like_level() }; } };
	auto f = p.get();
	p();
	return f;
}

Future<Level> LoadLevel(
	cref<LevelAsset> levelAsset_,
	Opt<LevelLoadOptions> options_
) noexcept {
	::hg::todo_panic();
}

Vector<Level> hg::GetLevels(
	cref<Universe> universe_,
	Opt<LevelQueryOptions> options_
) noexcept {

	const auto universe = universe_.unwrap();
	const auto levels = universe->getLevels();

	Vector<Level> result {};
	result.reserve(levels.size());
	for (const auto& level : levels) {
		result.emplace_back(clone(level));
	}
	return result;
}

Vector<Level> hg::GetLevelsAt(
	cref<Universe> universe_,
	const Vector3 location_,
	Opt<LevelQueryOptions> options_
) noexcept {

	const auto universe = GetUniverse().unwrap();
	const auto levels = universe->getLevels();

	Vector<Level> result {};
	for (const auto& level : levels) {
		if (level->getBounding().intersectsAaBb(location_)) {
			result.emplace_back(clone(level));
		}
	}
	return result;
}

Future<bool> SaveLevel(
	ref<LevelAsset> levelAsset_,
	Opt<LevelSaveOptions> options_
) noexcept {
	::hg::todo_panic();
}

#include <Engine.Assets.Type/Asset.hpp>
#include <Engine.Assets.Type/Universe/LevelAsset.hpp>
Future<bool> SaveLevel(
	cref<Level> level_,
	ref<LevelAsset> levelAsset_,
	Opt<LevelSaveOptions> options_ = None
) noexcept {
	::hg::todo_panic();
}

Future<bool> hg::Destroy(mref<Level> level_) noexcept {
	::hg::todo_panic();
}
