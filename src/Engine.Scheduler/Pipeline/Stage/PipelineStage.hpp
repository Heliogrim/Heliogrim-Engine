#pragma once
#include <Engine.Common/Collection/AssociativeKey.hpp>

#include "StageDispatcher.hpp"
#include "StageIdentifier.hpp"
#include "../StagePipeline.hpp"

namespace hg::engine::scheduler {
    class macro_novtable PipelineStage {
    public:
        using this_type = PipelineStage;

        using identifier_type = AssocKey<StageIdentifier>;
        using pipeline_handle_type = const non_owning_rptr<const StagePipeline>;

    protected:
        PipelineStage(
            cref<identifier_type> identifier_,
            pipeline_handle_type pipeline_
        ) noexcept;

    public:
        virtual ~PipelineStage() noexcept;

    protected:
        const identifier_type _stageIdentifier;

    public:
        [[nodiscard]] cref<identifier_type> stageIdentifier() const noexcept;

    protected:
        pipeline_handle_type _stagePipeline;

    public:
        [[nodiscard]] pipeline_handle_type stagePipeline() const noexcept;

    public:
        virtual void staticDispatch(const non_owning_rptr<const StageDispatcher> dispatcher_) = 0;

        virtual void dynamicDispatch(const non_owning_rptr<const StageDispatcher> dispatcher_) = 0;
    };
}
