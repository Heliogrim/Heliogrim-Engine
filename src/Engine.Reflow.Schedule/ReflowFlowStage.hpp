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
        void staticDispatch(const non_owning_rptr<const scheduler::StageDispatcher> dispatcher_) override;

        void dynamicDispatch(const non_owning_rptr<const scheduler::StageDispatcher> dispatcher_) override;
    };
}
