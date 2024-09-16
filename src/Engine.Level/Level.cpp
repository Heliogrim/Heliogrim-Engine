#include "Level.hpp"

#include <Engine.Common/Math/Coordinates.hpp>

using namespace hg::engine::core;
using namespace hg;

/**/

constexpr static auto level_root_bounding = ::hg::math::Bounding { math::vec3_zero, math::vec3_inf, math::vec3_inf.x };

/**/

bool Level::isRootLike() const noexcept {
	return _bounding.center() == level_root_bounding.center() &&
		_bounding.extent() == level_root_bounding.extent();
}

cref<math::Bounding> Level::getBounding() const noexcept {
	return _bounding;
}

void Level::setBounding(cref<math::Bounding> bounding_) noexcept {
	_bounding = bounding_;
}

void Level::addActor(mref<nmpt<Actor>> actor_) {
	std::ignore = _actors.emplace(std::move(actor_));
}

std::span<const nmpt<Actor>> Level::getActors() const noexcept {
	return std::span { _actors.values() };
}

void Level::removeActor(const nmpt<Actor> actor_) {
	std::ignore = _actors.erase(actor_);
}

Arci<Level> engine::core::make_root_like_level() {
	auto level = Arci<Level>::create();
	level->setBounding(level_root_bounding);
	return level;
}
