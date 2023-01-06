#pragma once
#include <Engine.Scheduler/Pipeline/Stage/PipelineStage.hpp>

namespace ember::engine::core::schedule {
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
        void staticDispatch(const non_owning_rptr<const scheduler::StageDispatcher> dispatcher_) override;

        void dynamicDispatch(const non_owning_rptr<const scheduler::StageDispatcher> dispatcher_) override;
    };
}
