#pragma once
#include <Engine.Scheduler/Pipeline/Stage/PipelineStage.hpp>

namespace hg::engine::input::schedule {
    class InputTickStage final :
        public scheduler::PipelineStage {
    public:
        using this_type = InputTickStage;

    public:
        InputTickStage(
            cref<scheduler::StageIdentifier> identifier_,
            pipeline_handle_type pipeline_
        );

        ~InputTickStage() override;

    public:
        void staticDispatch(const non_owning_rptr<const scheduler::StageDispatcher> dispatcher_) override;

        void dynamicDispatch(const non_owning_rptr<const scheduler::StageDispatcher> dispatcher_) override;
    };
}
