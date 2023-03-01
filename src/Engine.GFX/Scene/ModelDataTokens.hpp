#pragma once

#include "../Renderer/RenderDataToken.hpp"

namespace hg::engine::gfx::render {
    constexpr RenderDataToken CameraTransform {
        RenderDataToken::make(
            "CameraTransform"sv,
            RenderDataScope::eRenderPass,
            RenderDataSharing::eShared,
            RenderDataUsage::eSingle
        )
    };

    constexpr RenderDataToken StaticModelGeometry {
        RenderDataToken::make(
            "StaticModelGeometry"sv,
            RenderDataScope::eExternal,
            RenderDataSharing::eShared,
            RenderDataUsage::eSingle
        )
    };

    constexpr RenderDataToken StaticModelTransform {
        RenderDataToken::make(
            "StaticModelTransform"sv,
            RenderDataScope::eRenderPass,
            RenderDataSharing::eShared,
            RenderDataUsage::eModelInstance
        )
    };
}
