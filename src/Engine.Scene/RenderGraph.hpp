#pragma once

#include "Graph/MutableSceneGraph.hpp"

namespace ember::engine::scene {

    /**
     * Forward Declaration
     */
    class SceneNodeModel;

    using RenderGraph = SceneGraph<SceneNodeModel>;
}
