#include "SceneNodeModel.hpp"

using namespace hg::engine::gfx;
using namespace hg;

SceneNodeModel::SceneNodeModel(const ptr<SceneComponent> owner_) :
    InheritMeta(owner_),
    _sceneNodePath() {}
