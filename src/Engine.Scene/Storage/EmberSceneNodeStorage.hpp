#pragma once

#include "SceneNodeStorage.hpp"
#include "../Node/SceneNodeId.hpp"
#include "../Node/SceneNode.hpp"

namespace ember::engine::scene {

    class EmberSceneNodeStorage :
        public scene_node_storage<SceneNode, SceneNodeId> {};

}
