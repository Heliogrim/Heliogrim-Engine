#include "Level.hpp"

#include <Engine.ACS/ActorModule.hpp>
#include <Engine.ACS/Registry.hpp>
#include <Engine.Assets.Type/Universe/LevelAsset.hpp>
#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Resource.Archive/Archive.hpp>
#include <Engine.Scene/SceneBase.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Diagnostic/Codes.hpp>
#include <Engine.Serialization/Load/LoadContext.hpp>
#include <Engine.Serialization/Load/RootKind.hpp>
#include <Engine.Serialization/Load/Validate.hpp>
#include <Engine.Serialization/Structure/RootScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

import Engine.Common;

using namespace hg::engine::serialization;
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

void Level::setBounding(cref<math::Bounding> bounding_) noexcept {
	_bounding = bounding_;
}

Opt<ref<engine::core::Universe>> Level::getUniverse() const noexcept {
	return _universe;
}

void Level::setOwnerUniverse(mref<Opt<ref<engine::core::Universe>>> owner_) {
	_universe = ::hg::move(owner_);
}

bool Level::carriesUnknownData() const noexcept {
	return _unknownData;
}

void Level::markUnknownData() noexcept {
	_unknownData = true;
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

Result<void, engine::diag::Diagnostics> engine::core::checkSaveAllowed(ref<const Level> level_) {

	Vector<String> notes {};
	if (level_.carriesUnknownData()) {
		notes.emplace_back("level " + encodeGuid4228(level_.guid()) + ": records were skipped on load");
	}
	for (const auto actor : level_.getActors()) {
		if (actor->carriesUnknownData()) {
			notes.push_back(
				"level " + encodeGuid4228(level_.guid()) + " / actor " + encodeGuid4228(actor->guid()) +
				": fields or components were not understood on load"
			);
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
	out.emplace_back(::hg::move(refusal));
	return Unexpected<engine::diag::Diagnostics> { ::hg::move(out) };
}

template <>
void access::Structure<Level>::hydrate(cref<StructScopedSlot> record_, Level& target_);

Result<Arci<Level>, engine::diag::Diagnostics> engine::core::loadLevel(
	ref<engine::resource::Archive> archive_,
	const StringView url_,
	ref<engine::diag::DiagnosticSink> sink_,
	mref<Arci<Level>> into_
) {
	auto ctx = LoadContext { static_cast<u64>(std::max(archive_.totalSize(), 0LL)), String { url_ } };
	const auto valid = validate(archive_, 0LL, ctx).has_value();
	archive_.seek(0LL);

	auto level = into_ != nullptr ? ::hg::move(into_) : make_root_like_level();
	if (valid) {
		const auto root = RootScopedSlot { archive_, ScopedSlotStateFlag::eClean, &ctx };
		const auto kind = classifyRoot(root, ctx);
		if (kind == RootKind::eRecord) {
			access::Structure<Level>::hydrate(root.asStruct(), *level);

		} else if (kind.has_value()) {
			ctx.fail(ReadError { ReadError::Kind::eMalformed, root.slot()->getState().offset, 0LL, "level root must be a record" });
		}
	}

	auto finished = ctx.finish(sink_, std::format("while loading level '{}'", url_));
	if (not finished.has_value()) {
		// finish hands errors back without emitting them; the boundary reports them.
		for (const auto& diagnostic : finished.error()) {
			sink_.emit(engine::diag::Diagnostic { diagnostic });
		}
		return Unexpected<engine::diag::Diagnostics> { ::hg::move(finished.error()) };
	}

	return level;
}
