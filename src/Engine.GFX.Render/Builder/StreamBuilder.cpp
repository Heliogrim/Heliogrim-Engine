#include "StreamBuilder.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX.RenderGraph/RenderGraph.hpp>
#include <Engine.GFX.RenderPipeline/RenderPipeline.hpp>

#include "BuilderLayerVisitor.hpp"

using namespace hg::engine::gfx::render;
using namespace hg;

uptr<RenderPipeline> StreamBuilder::operator()(
    mref<uptr<RenderPipeline>> pipeline_,
    mref<uptr<RenderGraph>> nextGraph_
) const {

    auto visitor = BuilderLayerVisitor();
    nextGraph_->begin()->accept(visitor);

    Vector<smr<pipeline::Stage>> stages {};

    const auto& ordered = visitor.layers();
    for (const auto layer : ordered) {
        for (const auto node : layer) {

            // TODO: transform node to stage if required and push to set
            smr<pipeline::Stage> stage { nullptr };
            stages.push_back(_STD move(stage));

        }
    }

    // TODO: Rework to fetch the delta of the previous pipeline and the current
    // TODO: Introduce diffing mechanism to prevent excessive recreation

    /**/

    auto pipeline = make_uptr<RenderPipeline>();
    pipeline->setStages(_STD move(stages));

    return pipeline;

}
