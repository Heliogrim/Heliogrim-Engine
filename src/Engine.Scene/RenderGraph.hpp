#pragma once

#include "Graph/MutableSceneGraph.hpp"

/**
 * Forward Declaration
 */
namespace hg::engine::gfx {
    class SceneNodeModel;
}

namespace hg::engine::scene {
    /**
     * Attention: This will fail the concept assert cause `gfx::SceneNodeModel` based on this file only is just a incomplete type
     */

    //
    using RenderGraph = SceneGraph<gfx::SceneNodeModel>;
}
