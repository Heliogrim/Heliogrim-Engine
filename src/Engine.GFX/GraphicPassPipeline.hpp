#pragma once
#include <Engine.Common/Collection/Vector.hpp>

#include "GraphicPassPipelineStage.hpp"
#include "ModelPassProcessor.hpp"

namespace ember::engine::gfx {

    class GraphicPassPipeline {
    public:
        /**
         * Process the given processor_
         *
         *  If [processor_ != nullptr] use internal state of processor
         *  If [processor_ == nullptr] single invoke pipeline with nullptr
         *
         * @author Julius
         * @date 17.01.2021
         *
         * @param  processor_ The processor.
         * @param batch_ The recording command batch.
         */
        void process(ptr<const ModelPassProcessor> processor_, IN OUT ref<CommandBatch> batch_);

    private:
        /**
         * Collection of Stages
         */
        Vector<ptr<GraphicPassPipelineStage>> _stages;

    public:

        /**
         * Adds stage_ to the back of the pipeline stage chain
         *
         * @author Julius
         * @date 17.01.2021
         *
         * @param  stage_ The stage to add.
         */
        void add(cref<ptr<GraphicPassPipelineStage>> stage_);

        /**
         * Adds stage_ at the given index. Element after idx_ will be shifted
         *
         * @author Julius
         * @date 17.01.2021
         *
         * @param  stage_ The stage.
         * @param  idx_ Zero-based index of the.
         */
        void add(cref<ptr<GraphicPassPipelineStage>> stage_, u32 idx_);

        /**
         * Gets the stages
         *
         * @author Julius
         * @date 04.02.2021
         *
         * @returns A cref&lt;vector&lt;ptr&lt;GraphicPassPipelineStage&gt;&gt;&gt;
         */
        [[nodiscard]] cref<Vector<ptr<GraphicPassPipelineStage>>> stages() const noexcept;

        /**
         * Gets the stages
         *
         * @author Julius
         * @date 04.02.2021
         *
         * @returns A ref&lt;vector&lt;ptr&lt;GraphicPassPipelineStage&gt;&gt;&gt;
         */
        [[nodiscard]] ref<Vector<ptr<GraphicPassPipelineStage>>> stages() noexcept;
    };
}
