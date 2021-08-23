#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "GraphicPassStageContext.hpp"
#include "ProcessedModelBatch.hpp"

namespace ember::engine::gfx {

    class GraphicPassPipelineStage {
    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 12.02.2021
         */
        virtual ~GraphicPassPipelineStage() = default;

        /**
         * Setups this 
         *
         * @author Julius
         * @date 22.01.2021
         */
        virtual void setup() = 0;

        /**
         * Destroys this 
         *
         * @author Julius
         * @date 22.01.2021
         */
        virtual void destroy() noexcept = 0;

        /**
         * Checks whether the given batch should be processed in this stage
         *
         * @author Julius
         * @date 17.01.2021
         *
         * @param  batch_ The batch.
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] virtual bool check(ptr<const ProcessedModelBatch> batch_) noexcept = 0;

        /**
         * Preprocess for Batches.
         * 
         * @note Will be invoked once per GraphicPassPipeline invocation when first check is positive
         *
         * @author Julius
         * @date 12.02.2021
         *
         * @param  ctx_ The context.
         */
        virtual void before(cref<GraphicPassStageContext> ctx_) = 0;

        /**
         * Process the given batch_
         *
         * @author Julius
         * @date 17.01.2021
         *
         * @param  ctx_ The context.
         * @param  batch_ The batch.
         */
        virtual void process(cref<GraphicPassStageContext> ctx_, ptr<const ProcessedModelBatch> batch_) = 0;

        /**
         * Postprocess for Batches.
         * 
         * @ note Will be invoked once per GraphicPassPipeline invocation if any check for positive
         *
         * @author Julius
         * @date 12.02.2021
         *
         * @param  ctx_ The context.
         */
        virtual void after(cref<GraphicPassStageContext> ctx_) = 0;
    };
}
