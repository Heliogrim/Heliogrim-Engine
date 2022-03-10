#include "GraphicPassPipeline.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

using namespace ember::engine::gfx;
using namespace ember;

void GraphicPassPipeline::allocateWith(const ptr<const RenderInvocation> invocation_,
    const ptr<RenderInvocationState> state_) {
    /**
     *
     */
    for (auto* stage : _stages) {
        stage->allocateWith(invocation_, state_);
    }
}

void GraphicPassPipeline::freeWith(const ptr<const RenderInvocation> invocation_,
    const ptr<RenderInvocationState> state_) {
    /**
     *
     */
    for (auto it { _stages.rbegin() }; it != _stages.rend(); ++it) {
        (*it)->freeWith(invocation_, state_);
    }
}

void GraphicPassPipeline::process(ptr<const GraphicPassModelProcessor> processor_, ref<RenderInvocationState> state_,
    ref<CommandBatch> batch_) {

    SCOPED_STOPWATCH

    for (auto* stage : _stages) {

        GraphicPassStageContext ctx {
            batch_,
            state_
        };

        /**
         * If processor_ is nullptr, then execute pipeline with nullptr model
         */
        if (processor_ == nullptr && stage->check(nullptr)) {
            stage->before(ctx);
            stage->process(ctx, nullptr);
            stage->after(ctx);
            continue;
        }

        /**
         * If processor_ is not nullptr, move horizontal over stages and execute vertical models per stage
         *
         * // TODO: If large sequence, check whether we can divide object stack into parallel 
         */

        cref<Vector<ProcessedModelBatch>> models { processor_->models() };
        bool untouched = true;

        for (cref<ProcessedModelBatch> model : models) {
            /**
             * If not processable or effected, discard from stage
             */
            if (!stage->check(&model)) {
                continue;
            }

            /**
             * If model is first positive, prepare stage for processing
             */
            if (untouched) {
                stage->before(ctx);
                untouched = false;
            }

            /**
             *
             */
            stage->process(ctx, &model);
        }

        /**
         * If stage was effected by model collection, post process stage
         */
        if (!untouched) {
            stage->after(ctx);
        }
    }
}

void GraphicPassPipeline::add(cref<ptr<GraphicPassPipelineStage>> stage_) {
    _stages.push_back(stage_);
}

void GraphicPassPipeline::add(cref<ptr<GraphicPassPipelineStage>> stage_, u32 idx_) {
    const auto begin = _stages.begin();
    const auto at = begin + idx_;
    _stages.insert(at, stage_);
}

cref<Vector<ptr<GraphicPassPipelineStage>>> GraphicPassPipeline::stages() const noexcept {
    return _stages;
}

ref<Vector<ptr<GraphicPassPipelineStage>>> GraphicPassPipeline::stages() noexcept {
    return _stages;
}
