#include "Level.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Assets.Type/Universe/LevelAsset.hpp>
#include <Engine.Async/Await/Promise.hpp>
#include <Engine.Common/Discard.hpp>
#include <Engine.Common/Move.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Level/Level.hpp>

#include "Universe.hpp"
#include "../Asset/UniverseAssetHandles.hpp"

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

ptr<Actor> Level::addActor(mref<VolatileActor<>> actor_) {
	auto* const ref = actor_.get();
	_internal->addActor(::hg::move(actor_));
	return ref;
}

Opt<VolatileActor<>> Level::removeActor(ptr<Actor> actor_) {
	return _internal->removeActor(actor_);
}

void Level::dropActor(mref<ptr<Actor>> actor_) {
	auto removed = _internal->removeActor(actor_);
	::hg::discard(removed);
}

Future<Level> hg::CreateLevel() noexcept {
	auto p = concurrent::promise<Level> { [] { return Level { engine::core::make_root_like_level() }; } };
	auto f = p.get();
	p();
	return f;
}

Future<Level> LoadLevel(
	ref<const LevelAssetHandle> levelAsset_,
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
	ref<LevelAssetHandle> levelAsset_,
	Opt<LevelSaveOptions> options_
) noexcept {
	::hg::todo_panic();
}

/* TODO: Move */
#include <Engine.Assets.Type/Asset.hpp>
#include <Engine.Assets.Type/Universe/LevelAsset.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Resource.Archive/BufferArchive.hpp>
#include <Engine.Resource.Save/StorePipeline.hpp>
#include <Engine.Serialization/Archive/StructuredArchive.hpp>
#include <Engine.Storage.Registry/IStorageRegistry.hpp>
#include <Engine.Storage/StorageModule.hpp>
/**/

Future<bool> SaveLevel(
	cref<Level> level_,
	ref<LevelAssetHandle> levelAsset_,
	Opt<LevelSaveOptions> options_ = None
) noexcept {

	/**
	 * TODO: Checklist
	 * 1. Check level_ validity and integrity
	 * 2. Check levelAsset_ validity
	 * Note: We expect the asset to have a valid storage url
	 * 3. Check whether we can IPS or have to buffer.
	 *	(3.1) If buffer, acquire temporary resources
	 * 4. Request storage object with write permission from storage sub-system
	 * 7.
	 */

	::hg::assertrt(level_.valid());
	::hg::assertrt(levelAsset_.isValid());
	::hg::assertrt(not levelAsset_.getAssetRef()->getAssetStorageUrl().encode().empty());

	::hg::todo_panic();
}

Future<bool> hg::Destroy(mref<Level> level_) noexcept {
	::hg::todo_panic();
}
