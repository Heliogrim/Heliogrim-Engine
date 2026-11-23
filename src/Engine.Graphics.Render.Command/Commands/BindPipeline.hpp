#pragma once

#include "Engine.Graphics.Render.Command/RenderCommand.hpp"
#include "Engine.Graphics.Render.Command/RenderResource.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Accel.Pipeline;
#endif

namespace hg::engine::render::cmd {
    enum class BindPipelineFeatureSet {
        eCompute,
        eGraphics
    };

    class BindPipelineRCmd :
        public RenderCommand {
    public:
        using this_type = BindPipelineRCmd;

    public:
        constexpr BindPipelineRCmd(
            mref<smr<const accel::AccelerationPipeline>> pipeline_,
            const BindPipelineFeatureSet featureSet_
        ) noexcept :
            pipeline(std::move(pipeline_)),
            featureSet(featureSet_) {}

        ~BindPipelineRCmd() noexcept override = default;

    private:
    public:
        smr<const accel::AccelerationPipeline> pipeline;
        BindPipelineFeatureSet featureSet;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}
