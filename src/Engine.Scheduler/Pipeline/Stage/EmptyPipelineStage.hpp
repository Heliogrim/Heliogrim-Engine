#pragma once

#include "PipelineStage.hpp"

namespace hg::engine::scheduler {
    class EmptyPipelineStage final :
        public PipelineStage {
    public:
        using this_type = EmptyPipelineStage;

    public:
        EmptyPipelineStage(cref<identifier_type> identifier_, pipeline_handle_type pipeline_);

        ~EmptyPipelineStage() noexcept override;

    public:
        void staticDispatch(const non_owning_rptr<const StageDispatcher> dispatcher_) override;

        void dynamicDispatch(const non_owning_rptr<const StageDispatcher> dispatcher_) override;
    };
}
