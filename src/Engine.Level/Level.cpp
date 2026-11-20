#include "Level.hpp"

#include <Engine.ACS/ActorModule.hpp>
#include <Engine.ACS/Registry.hpp>
#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Assets.Type/Universe/LevelAsset.hpp>
#include <Engine.Common/GuidFormat.hpp>
#include <Engine.Common/Math/Coordinates.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Scene/SceneBase.hpp>
#include <Engine.Serialization/Diagnostic/Codes.hpp>

using namespace hg::engine::core;
using namespace hg;

/**/

constexpr static auto level_root_bounding = ::hg::math::Bounding { math::vec3_zero, math::vec3_inf, math::vec3_inf.x };

/**/

namespace {
	[[nodiscard]] ref<engine::acs::Registry> actorRegistry() noexcept {
		return *engine::Engine::getEngine()->getActors()->getRegistry();
	}
}

/**/

bool Level::carriesUnknownData() const noexcept {
	return _unknownData;
}

void Level::markUnknownData() noexcept {
	_unknownData = true;
}

Result<void, engine::diag::Diagnostics> engine::core::checkSaveAllowed(ref<const Level> level_) {

	Vector<String> notes {};
	if (level_.carriesUnknownData()) {
		notes.push_back("level " + encodeGuid4228(level_.guid()) + ": records were skipped on load");
	}
	for (const auto actor : level_.getActors()) {
		if (actor->carriesUnknownData()) {
			notes.push_back("level " + encodeGuid4228(level_.guid()) + " / actor " + encodeGuid4228(actor->guid()) + ": fields or components were not understood on load");
		}
	}

	if (notes.empty()) {
		return {};
	}

	engine::diag::Diagnostic refusal = engine::diag::Diagnostic::error(
		engine::serialization::codes::ser("save-refused"),
		"level holds data this build did not understand; saving would drop it"
	);
	for (auto& note : notes) {
		refusal.addNote(::hg::move(note));
	}

	engine::diag::Diagnostics out {};
	out.push_back(::hg::move(refusal));
	return Unexpected<engine::diag::Diagnostics> { ::hg::move(out) };
}

Level::Level(mref<LevelGuid> guid_) noexcept :
	_guid(::hg::move(guid_)) {}

Level::Level(ref<const assets::LevelAsset> asset_) noexcept :
	_guid(asset_.getAssetGuid()) {}

Level::~Level() {
	for (const auto actor : _actors) {
		VolatileActor<>::destroy(actor);
	}
	_actors.clear();

	// Frees the level's chunks and handle pages as a whole.
	if (_space != acs::detachedSpace) {
		actorRegistry().releaseSpace(_space);
	}
}

bool Level::isRootLike() const noexcept {
	return _bounding.center() == level_root_bounding.center() &&
		_bounding.extent() == level_root_bounding.extent();
}

ref<const LevelGuid> Level::guid() const noexcept {
	return _guid;
}

ref<const AssetGuid> Level::assetGuid() const noexcept {
	return reinterpret_cast<ref<const AssetGuid>>(_guid);
}

cref<math::Bounding> Level::getBounding() const noexcept {
	return _bounding;
}

Opt<ref<engine::core::Universe>> Level::getUniverse() const noexcept {
	return _universe;
}

void Level::setBounding(cref<math::Bounding> bounding_) noexcept {
	_bounding = bounding_;
}

void Level::setOwnerUniverse(mref<Opt<ref<engine::core::Universe>>> owner_) {
	_universe = ::hg::move(owner_);
}

engine::acs::SpaceId Level::space() {
	if (_space == acs::detachedSpace) {
		_space = actorRegistry().acquireSpace();
	}
	return _space;
}

ptr<Actor> Level::addActor(mref<VolatileActor<>> actor_) {

	auto* actor = actor_.release();
	if (actor->getArchetype() != nullptr) {
		actor = actorRegistry().moveActor(actor, space());
	}

	_actors.emplace(actor);
	actor->setOwnerLevel(Some(*this));

	if (not _universe.is_null()) [[likely]] {
		const auto ctx = _universe->getScene()->registerContext();
		actor->registerComponents(*_universe, *ctx.get());
	}
	return actor;
}

std::span<const owner_ptr<Actor>> Level::getActors() const noexcept {
	return std::span { _actors.values() };
}

Opt<VolatileActor<>> Level::removeActor(const nmpt<Actor> actor_) {
	if (not _actors.contains(actor_.get())) [[unlikely]] {
		return None;
	}

	const auto attachedTo = actor_->getUniverse();
	if (attachedTo != None) {
		actor_->unregisterComponents(*attachedTo->getScene()->registerContext());
	}

	_actors.erase(actor_.get());
	actor_->setOwnerLevel(None);

	auto* actor = actor_.get();
	if (actor->getArchetype() != nullptr) {
		actor = actorRegistry().moveActor(actor, acs::detachedSpace);
	}
	return SomeVal<VolatileActor<>>(::hg::move(actor));
}

Arci<Level> engine::core::make_root_like_level() {
	auto level = Arci<Level>::create(generate_level_guid());
	level->setBounding(level_root_bounding);
	return level;
}
