#include "BaseBuilder.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.GFX.RenderGraph/RenderGraph.hpp>
#include <Engine.GFX.RenderPipeline/RenderPipeline.hpp>
#include <Engine.GFX.RenderPipeline/Stage/Stage.hpp>

#include "BuilderProgGenVisitor.hpp"
#include "BuilderScanVisitor.hpp"
#include "BuilderLayerVisitor.hpp"

using namespace hg::engine::gfx::render;
using namespace hg;

uptr<RenderPipeline> BaseBuilder::operator()(mref<nmpt<RenderGraph>> graph_) const {

    auto scanner = BuilderScanVisitor();
    graph_->begin()->accept(scanner);

    auto layerer = BuilderLayerVisitor();
    layerer.setNodeRcDep(scanner.extractNodeRcMap());
    graph_->begin()->accept(layerer);

    auto gen = BuilderProgGenVisitor();
    gen.setNodeRcDep(layerer.extractNodeRcDep());

    Vector<smr<pipeline::Stage>> stages {};

    const auto& ordered = layerer.layers();
    for (const auto layer : ordered) {
        for (const auto node : layer) {

            // TODO: transform node to stage if required and push to set
            node->accept(gen);
            auto stage = gen.findStage(node.get());

            if (stage.empty()) {
                continue;
            }

            stages.push_back(_STD move(stage));
        }
    }

    assert(stages.size() == gen.getNodeStageMapping().size());

    /**/

    auto pipeline = make_uptr<RenderPipeline>();
    pipeline->setStages(_STD move(stages));

    return pipeline;
}
