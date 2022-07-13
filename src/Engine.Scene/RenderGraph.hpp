#pragma once

#include "Graph/MutableSceneGraph.hpp"

/**
 * Forward Declaration
 */
namespace ember::engine::gfx {
    class SceneNodeModel;
}

namespace ember::engine::scene {

    /**
     * Attention: This will fail the concept assert cause `gfx::SceneNodeModel` based on this file only is just a incomplete type
     */

    //
    using RenderGraph = SceneGraph<gfx::SceneNodeModel>;
}
