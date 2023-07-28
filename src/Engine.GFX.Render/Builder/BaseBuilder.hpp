#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX.RenderGraph/__fwd.hpp>
#include <Engine.GFX.RenderPipeline/__fwd.hpp>

namespace hg::engine::gfx::render {
    class BaseBuilder final {
    public:
        using this_type = BaseBuilder;

    public:
        [[nodiscard]] uptr<RenderPipeline> operator()(mref<nmpt<RenderGraph>> graph_) const;
    };
}
