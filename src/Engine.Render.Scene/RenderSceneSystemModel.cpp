#include "RenderSceneSystemModel.hpp"

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
