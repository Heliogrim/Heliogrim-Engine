#pragma once

#include "Graph/MutableSceneGraph.hpp"

/**
 * Forward Declaration
 */
namespace hg::engine::gfx::scene {
    class SceneModel;
}

namespace hg::engine::scene {
    /**
     * Attention: This will fail the concept assert cause `gfx::SceneNodeModel` based on this file only is just a incomplete type
     */

    //
    using RenderSceneGraph = SceneGraph<gfx::scene::SceneModel>;
}
