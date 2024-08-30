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
        void staticDispatch(cref<scheduler::StageDispatcher> dispatcher_) override;

        void dynamicDispatch(cref<scheduler::StageDispatcher> dispatcher_) override;
    };
}
