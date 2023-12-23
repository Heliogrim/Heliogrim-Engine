#pragma once

#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "RenderGraph.hpp"

namespace hg::engine::gfx {
    class SceneResourcePool;
}

namespace hg::engine::scene {
    class __declspec(novtable) IRenderScene {
    public:
        using this_type = IRenderScene;

    public:
        [[nodiscard]] virtual const ptr<RenderSceneGraph> renderGraph() noexcept = 0;

        [[nodiscard]] virtual nmpt<gfx::SceneResourcePool> getSceneResourcePool() const noexcept = 0;
    };
}
