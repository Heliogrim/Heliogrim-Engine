#pragma once
#include <Engine.Accel.Pipeline/AccelerationPipeline.hpp>

#include "Engine.GFX.Render.Command/RenderCommand.hpp"
#include "Engine.GFX.Render.Command/RenderResource.hpp"

namespace hg::engine::gfx::render::cmd {
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
            pipeline(_STD move(pipeline_)),
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
