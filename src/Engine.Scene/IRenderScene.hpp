#pragma once
#include "RenderGraph.hpp"

namespace hg::engine::scene {
    class __declspec(novtable) IRenderScene {
    public:
        using this_type = IRenderScene;

    public:
        [[nodiscard]] virtual const ptr<RenderGraph> renderGraph() noexcept = 0;
    };
}
