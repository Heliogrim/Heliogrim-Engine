#include "BaseBuilder.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.GFX.RenderGraph/CompileGraph.hpp>
#include <Engine.GFX.RenderGraph/RuntimeGraph.hpp>
#include <Engine.GFX.RenderGraph/Debug/DebugVisitor.hpp>
#include <Engine.GFX.RenderGraph/Resolver/Resolver.hpp>
#include <Engine.GFX.RenderGraph/Resolver/ResolverOptions.hpp>
#include <Engine.GFX.RenderPipeline/RenderPipeline.hpp>
#include <Engine.GFX.RenderPipeline/Stage/Stage.hpp>

#include "BuilderProgGenVisitor.hpp"
#include "BuilderScanVisitor.hpp"
#include "BuilderLayerVisitor.hpp"
#include "CompileVisitor.hpp"

using namespace hg::engine::gfx::render;
using namespace hg;

uptr<graph::RuntimeGraph> BaseBuilder::compile(mref<nmpt<graph::CompileGraph>> graph_) const {

    auto runtime = make_uptr<graph::RuntimeGraph>();

    /**/

    auto compiler = CompileVisitor(runtime.get());
    graph_->update(compiler);

    /**/

    graph::Resolver resolver {};
    graph::ResolverOptions options {
        .flags = graph::ResolverPassFlags {} | graph::ResolverPassFlagBits::eBasicLayout |
        graph::ResolverPassFlagBits::eValidate
    };

    runtime = resolver(_STD move(runtime), _STD move(options));

    /**/

    graph::DebugVisitor debug {};
    runtime->update(debug);

    /**/

    return runtime;
}

uptr<RenderPipeline> BaseBuilder::operator()(mref<nmpt<graph::CompileGraph>> graph_) const {

    auto runtime = compile(_STD move(graph_));

    /**/

    auto scanner = BuilderScanVisitor();
    runtime->begin()->accept(scanner);

    auto layerer = BuilderLayerVisitor();
    layerer.setNodeRcDep(scanner.extractNodeRcMap());
    runtime->begin()->accept(layerer);

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
