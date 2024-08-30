#pragma once
#include <Engine.Scheduler/Pipeline/Stage/PipelineStage.hpp>

namespace hg::engine::reflow::schedule {
    class ReflowFlowStage final :
        public scheduler::PipelineStage {
    public:
        using this_type = ReflowFlowStage;

    public:
        ReflowFlowStage(
            cref<scheduler::StageIdentifier> identifier_,
            pipeline_handle_type pipeline_
        );

        ~ReflowFlowStage() override;

    public:
        void staticDispatch(cref<scheduler::StageDispatcher> dispatcher_) override;

        void dynamicDispatch(cref<scheduler::StageDispatcher> dispatcher_) override;
    };
}
