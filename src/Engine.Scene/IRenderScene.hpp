#pragma once
#include "Graph/SceneGraph.hpp"

namespace ember::engine::scene {

    class __declspec(novtable) IRenderScene {
    public:
        using this_type = IRenderScene;

    public:
        [[nodiscard]] virtual const ptr<SceneGraph> renderGraph() noexcept = 0;
    };
}
