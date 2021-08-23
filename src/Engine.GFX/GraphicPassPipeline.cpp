#include "GraphicPassPipeline.hpp"

using namespace ember::engine::gfx;
using namespace ember;

void GraphicPassPipeline::process(ptr<const ModelPassProcessor> processor_, ref<CommandBatch> batch_) {

    for (auto* stage : _stages) {

        GraphicPassStageContext ctx {
            batch_
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

        vector<ProcessedModelBatch> dummy { 0 };
        bool untouched = true;

        for (cref<ProcessedModelBatch> model : /*processor_.models()*/dummy) {
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

cref<vector<ptr<GraphicPassPipelineStage>>> GraphicPassPipeline::stages() const noexcept {
    return _stages;
}

ref<vector<ptr<GraphicPassPipelineStage>>> GraphicPassPipeline::stages() noexcept {
    return _stages;
}
