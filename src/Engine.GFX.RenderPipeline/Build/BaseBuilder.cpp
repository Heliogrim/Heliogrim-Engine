#include "BaseBuilder.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "GraphScanner.hpp"
#include "GraphStager.hpp"
#include "ProgressiveGenerator.hpp"
#include "../Stage/Stage.hpp"

using namespace hg::engine::gfx::render::pipeline;
using namespace hg::engine::gfx::render;
using namespace hg;

/**/

static void resolveExecutionDependencies(
    mref<DenseMap<ptr<const graph::Node>, DenseSet<ptr<const graph::Node>>>> knownDependencies_,
    cref<DenseMap<ptr<const graph::Node>, smr<Stage>>> nodeToStage_,
    cref<Vector<smr<Stage>>> stageOrder_
);

/**/

uptr<RenderPipeline> BaseBuilder::operator()(mref<uptr<graph::RuntimeGraph>> runtimeGraph_) const {

    auto scanner = GraphScanner();
    auto stager = GraphStager();
    auto pg = ProgressiveGenerator();

    runtimeGraph_->begin()->accept(scanner);
    stager.storeKnownDependencies(scanner.extractNodeDependencies());
    runtimeGraph_->begin()->accept(stager);
    pg.storeKnownDependencies(stager.extractKnownDependencies());

    /**/

    Vector<smr<Stage>> pipeStages {};

    const auto graphStages = stager.extractStages();
    for (const auto& graphStage : graphStages) {
        for (const auto& node : graphStage) {

            node->accept(pg);
            auto pipeStage = pg.findStage(node.get());

            if (pipeStage.empty()) {
                // Could happen due to missing translation or unmaterialized nodes like AnchorNode
                continue;
            }

            pipeStages.push_back(_STD move(pipeStage));
        }
    }

    const auto nodeToStage = pg.extractProgressiveMapping();
    assert(pipeStages.size() == nodeToStage.size());

    resolveExecutionDependencies(pg.extractKnownDependencies(), nodeToStage, pipeStages);

    /**/

    auto pipeline = make_uptr<RenderPipeline>();
    pipeline->setStages(_STD move(pipeStages));

    return pipeline;
}

/**/
void resolveExecutionDependencies(
    mref<DenseMap<ptr<const graph::Node>, DenseSet<ptr<const graph::Node>>>> knownDependencies_,
    cref<DenseMap<ptr<const graph::Node>, smr<Stage>>> nodeToStage_,
    cref<Vector<smr<Stage>>> stageOrder_
) {

    /**
     * Warning: This dependency resolving is extremly costly
     *
     * Note: N_stages * N_dep * k_ps ~> O(N^2 * k_ps)
     */

    for (const auto& [node, stage] : nodeToStage_) {

        auto depIt = knownDependencies_.find(node);
        if (depIt == knownDependencies_.end() || depIt->second.empty()) {
            continue;
        }

        auto distance = static_cast<Vector<smr<Stage>>::difference_type>(stageOrder_.size());
        smr<Stage> latest {};

        for (const auto& known : depIt->second) {

            const auto knownStageIt = nodeToStage_.find(known);
            if (knownStageIt == nodeToStage_.end()) {
                continue;
            }

            const auto stageIt = _STD ranges::find(stageOrder_, knownStageIt->second);
            const auto dist = _STD distance(stageOrder_.begin(), stageIt);

            if (dist < distance) {
                distance = dist;
                latest = clone(knownStageIt->second);
            }
        }

        if (latest) {
            stage->setExecStateCnd(&latest->getExecState().state);
        }
    }

}
