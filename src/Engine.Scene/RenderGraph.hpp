#pragma once

#include "Graph/MutableSceneGraph.hpp"

/**
 * Forward Declaration
 */
namespace ember::engine::gfx {
    class SceneNodeModel;
}

namespace ember::engine::scene {

    using RenderGraph = SceneGraph<gfx::SceneNodeModel>;
}
