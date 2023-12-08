#pragma once

#include <Engine.GFX.Renderer/RenderDataToken.hpp>

namespace hg::engine::render {
    constexpr RenderDataToken PerformanceQuery {
        RenderDataToken::make(
            "PerformanceQuery"sv,
            RenderDataScope::eRenderPass,
            RenderDataSharing::eDedicated,
            RenderDataUsage::ePerRenderNode
        )
    };
}
