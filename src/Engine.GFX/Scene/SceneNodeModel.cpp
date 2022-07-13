#include "SceneNodeModel.hpp"

using namespace ember::engine::gfx;
using namespace ember;

SceneNodeModel::SceneNodeModel(const ptr<SceneComponent> owner_) :
    scene::SceneNodeModel(owner_),
    _sceneNodePath() {}
