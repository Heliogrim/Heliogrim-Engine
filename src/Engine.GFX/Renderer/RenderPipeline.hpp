#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "RenderContext.hpp"
#include "../GraphicPass/GraphicPass.hpp"

namespace ember::engine::gfx {

    class RenderPipeline {
    public:
        using this_type = RenderPipeline;

    public:
        RenderPipeline();

        RenderPipeline(cref<this_type>) = delete;

        RenderPipeline(mref<this_type>) = delete;

        ~RenderPipeline();

    public:
        void destroy();

        void setup();

    private:
        Vector<ptr<GraphicPass>> _passes;

    public:
        bool defineGraphicPass(const u8 graphicPassIndex_, const ptr<GraphicPass> graphicPass_);

    public:
        void process(const ptr<const RenderContext> ctx_, ref<CommandBatch> batch_) const;

    public:
        void allocateWith(const ptr<const RenderInvocation> invocation_,
            _Inout_ const ptr<RenderInvocationState> state_);

        void freeWith(const ptr<const RenderInvocation> invocation_, _Inout_ const ptr<RenderInvocationState> state_);
    };
}
