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
