#pragma once

#include "Graph/MutableSceneGraph.hpp"

/**
 * Forward Declaration
 */
namespace ember {
    class ActorComponent;
}

namespace ember::engine::scene {

    using RenderGraph = SceneGraph<ActorComponent>;
}
