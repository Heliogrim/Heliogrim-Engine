#pragma once
#include <Engine.Scheduler/Pipeline/Stage/PipelineStage.hpp>

namespace ember::engine::gfx {
    class RenderTarget;
}

namespace ember::engine::gfx::schedule {
    class RenderTickStage final :
        public scheduler::PipelineStage {
    public:
        using this_type = RenderTickStage;

    public:
        RenderTickStage(
            cref<scheduler::StageIdentifier> identifier_,
            pipeline_handle_type pipeline_
        );

        ~RenderTickStage() override;

    public:
        void staticDispatch(const non_owning_rptr<const scheduler::StageDispatcher> dispatcher_) override;

        void dynamicDispatch(const non_owning_rptr<const scheduler::StageDispatcher> dispatcher_) override;

    private:
        void tickTargets();

        void invokeRenderTarget(cref<sptr<RenderTarget>> target_) const;
    };
}
