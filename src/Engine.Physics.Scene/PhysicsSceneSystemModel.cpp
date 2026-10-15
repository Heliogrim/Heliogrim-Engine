#include "PhysicsSceneSystemModel.hpp"

using namespace hg::engine::physics;
using namespace hg;

PhysicsSceneSystemModel::PhysicsSceneSystemModel(ref<SceneComponent> owner_) noexcept :
	InheritBase(),
	_owner(std::addressof(owner_)) {}

const nmpt<SceneComponent> PhysicsSceneSystemModel::owner() const noexcept {
	return _owner;
}
