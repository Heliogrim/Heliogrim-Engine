#pragma once
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.RenderGraph/__fwd.hpp>
#include <Engine.GFX.RenderPipeline/__fwd.hpp>

#include "RenderPassResult.hpp"

namespace hg::engine::gfx::render {
    class RenderPass final {
    public:
        friend class Renderer;

    public:
        using this_type = RenderPass;

    public:
        RenderPass(
            u32 runtimeVersion_,
            mref<uptr<RenderPipeline>> pipeline_,
            mref<uptr<pipeline::State>> state_
        ) noexcept;

        RenderPass(cref<this_type>) = delete;

        RenderPass(mref<this_type>) = delete;

        ~RenderPass() noexcept;

    private:
        _STD atomic_uint_fast32_t _rtVer;

        uptr<RenderPipeline> _pipeline;
        uptr<pipeline::State> _state;

    protected:
        bool alloc();

        bool realloc();

        bool free();

    public:
        RenderPassResult operator()();
    };
}
