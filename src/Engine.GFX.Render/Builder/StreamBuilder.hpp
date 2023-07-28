#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.GFX.RenderGraph/__fwd.hpp>
#include <Engine.GFX.RenderPipeline/__fwd.hpp>

namespace hg::engine::gfx::render {
    class StreamBuilder final {
    public:
        using this_type = StreamBuilder;

    public:
        [[nodiscard]] uptr<RenderPipeline> operator()(
            mref<uptr<RenderPipeline>> pipeline_,
            mref<uptr<RenderGraph>> nextGraph_
        ) const;
    };
}
