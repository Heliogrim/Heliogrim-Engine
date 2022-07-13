#include "SceneNodeModel.hpp"

using namespace ember::engine::scene;
using namespace ember;

SceneNodeModel::SceneNodeModel(const ptr<SceneComponent> owner_) :
    _owner(owner_),
    _boundary() {}

SceneNodeModel::~SceneNodeModel() noexcept = default;

const ptr<SceneComponent> SceneNodeModel::owner() const noexcept {
    return _owner;
}

cref<math::Bounding> SceneNodeModel::getBoundaries() const noexcept {
    return _boundary;
}

bool SceneNodeModel::markedAsDeleted() const noexcept {
    return _markAsDeleted.test(_STD memory_order::consume);
}

bool SceneNodeModel::markAsDeleted() noexcept {
    return _markAsDeleted.test_and_set(_STD memory_order::release);
}
