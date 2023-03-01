#pragma once

#include "../Renderer/RenderDataToken.hpp"

namespace hg::engine::gfx::render {
    constexpr RenderDataToken PerformanceQuery {
        RenderDataToken::make(
            "PerformanceQuery"sv,
            RenderDataScope::eRenderPass,
            RenderDataSharing::eDedicated,
            RenderDataUsage::ePerRenderNode
        )
    };
}
