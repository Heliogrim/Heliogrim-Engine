#pragma once

#include <Engine.Common/Hash/Fnv-1a.hpp>

namespace hg::engine::gfx::render {
    enum class RenderDataScope : u8 {
        eExternal,
        eRenderPass
        /*
        eStagePass,
        eStageNode,
        eModelType,
        eModelInstance
         */
    };

    enum class RenderDataSharing : u8 {
        eDedicated,
        eShared
    };

    enum class RenderDataUsage : u8 {
        eWhole,
        eSingle = eWhole,
        // e.g. camera transform
        ePerRenderNode,
        // e.g. Performance Query struct per RenderNode
        eModelInstance// e.g. static model transforms
    };

    struct RenderDataToken {
        u64 token;

        RenderDataScope scope;
        RenderDataSharing sharing;
        RenderDataUsage usage;

        [[nodiscard]] constexpr static RenderDataToken make(
            cref<string_view> token_,
            const RenderDataScope scope_ = RenderDataScope::eExternal,
            const RenderDataSharing sharing_ = RenderDataSharing::eShared,
            const RenderDataUsage usage_ = RenderDataUsage::eWhole
        ) noexcept {
            return RenderDataToken {
                hash::fnv1a(token_),
                scope_,
                sharing_,
                usage_
            };
        }
    };
}
