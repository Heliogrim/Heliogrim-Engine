#include "SceneNodeModel.hpp"

using namespace hg::engine::gfx;
using namespace hg;

SceneNodeModel::SceneNodeModel(const ptr<SceneComponent> owner_) :
    scene::SceneNodeModel(owner_),
    _sceneNodePath() {}
