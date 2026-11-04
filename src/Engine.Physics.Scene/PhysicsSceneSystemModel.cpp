#include "PhysicsSceneSystemModel.hpp"

#include <Engine.ACS/Cluster/RelocationTable.hpp>

using namespace hg::engine::physics;
using namespace hg;

PhysicsSceneSystemModel::PhysicsSceneSystemModel(ref<SceneComponent> owner_) noexcept :
	InheritBase(),
	_owner(std::addressof(owner_)) {}

const nmpt<SceneComponent> PhysicsSceneSystemModel::owner() const noexcept {
	return _owner;
}

void PhysicsSceneSystemModel::relocate(ref<const acs::RelocationTable> table_) noexcept {
	if (auto* const owner = table_.lookup(reinterpret_cast<const std::byte*>(_owner.get())); owner != nullptr) {
		_owner = nmpt<SceneComponent> { reinterpret_cast<ptr<SceneComponent>>(owner) };
	}
}
