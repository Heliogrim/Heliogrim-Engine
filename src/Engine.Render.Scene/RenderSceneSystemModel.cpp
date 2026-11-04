#include "RenderSceneSystemModel.hpp"

#include <Engine.ACS/Cluster/RelocationTable.hpp>

using namespace hg::engine::render;
using namespace hg;

RenderSceneSystemModel::RenderSceneSystemModel(const nmpt<SceneComponent> owner_) noexcept :
	InheritBase(),
	_owner(owner_),
	_markAsDeleted(false) {}

const nmpt<SceneComponent> RenderSceneSystemModel::owner() const noexcept {
	return _owner;
}

bool RenderSceneSystemModel::markedAsDeleted() const noexcept {
	return _markAsDeleted;
}

void RenderSceneSystemModel::markAsDeleted() noexcept {
	_markAsDeleted = true;
}

void RenderSceneSystemModel::relocate(ref<const acs::RelocationTable> table_) noexcept {
	if (auto* const owner = table_.lookup(reinterpret_cast<const std::byte*>(_owner.get())); owner != nullptr) {
		_owner = nmpt<SceneComponent> { reinterpret_cast<ptr<SceneComponent>>(owner) };
	}
}
