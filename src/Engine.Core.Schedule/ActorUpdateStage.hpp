#pragma once
#include <Engine.Scheduler/Pipeline/Stage/PipelineStage.hpp>

namespace hg::engine::core::schedule {
    class ActorUpdateStage final :
        public scheduler::PipelineStage {
    public:
        using this_type = ActorUpdateStage;

    public:
        ActorUpdateStage(
            cref<scheduler::StageIdentifier> identifier_,
            pipeline_handle_type pipeline_
        );

        ~ActorUpdateStage() override;

    public:
        void staticDispatch(cref<scheduler::StageDispatcher> dispatcher_) override;

        void dynamicDispatch(cref<scheduler::StageDispatcher> dispatcher_) override;
    };
}
