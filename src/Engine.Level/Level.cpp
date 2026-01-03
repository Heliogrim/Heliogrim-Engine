#include "Level.hpp"

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Assets.Type/Universe/LevelAsset.hpp>
#include <Engine.Common/Math/Coordinates.hpp>

using namespace hg::engine::core;
using namespace hg;

/**/

constexpr static auto level_root_bounding = ::hg::math::Bounding { math::vec3_zero, math::vec3_inf, math::vec3_inf.x };

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

void Level::addActor(mref<VolatileActor<>> actor_) {
	_actors.emplace(actor_.release());
}

std::span<const owner_ptr<Actor>> Level::getActors() const noexcept {
	return std::span { _actors.values() };
}

Opt<VolatileActor<>> Level::removeActor(const nmpt<Actor> actor_) {
	if (not _actors.contains(actor_.get())) [[unlikely]] {
		return None;
	}
	_actors.erase(actor_.get());
	return Some<VolatileActor<>>(actor_.get());
}

Arci<Level> engine::core::make_root_like_level() {
	auto level = Arci<Level>::create(generate_level_guid());
	level->setBounding(level_root_bounding);
	return level;
}
