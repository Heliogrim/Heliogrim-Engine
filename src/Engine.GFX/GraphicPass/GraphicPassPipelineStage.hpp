#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "GraphicPassStageContext.hpp"
#include "ProcessedModelBatch.hpp"
#include "../Renderer/RenderInvocation.hpp"
#include "../Renderer/RenderContext.hpp"

namespace ember::engine::gfx {

    class __declspec(novtable) GraphicPassPipelineStage {
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

    public:
        /**
         * Allocates required resources for this stage at the given state
         *
         * @author Julius
         * @date 04.03.2022
         *
         * @param invocation_ The RenderInvocation used as source data
         * @param state_ The RenderInvocationState where to allocate the resources
         */
        virtual void allocateWith(const ptr<const RenderInvocation> invocation_,
            const ptr<RenderInvocationState> state_) = 0;

        /**
         * Frees the given resources allocated by this stage from the given state
         *
         * @author Julius
         * @date 04.03.2022
         *
         * @param invocation_ The RenderInvocation the allocated data is based on
         * @param state_ The RenderInvocationState where to free the resources from
         */
        virtual void freeWith(const ptr<const RenderInvocation> invocation_,
            const ptr<RenderInvocationState> state_) = 0;

    public:
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
         * @param  ctx_ The RenderContext containing the resources, scene, camera and target data.
         * @param  stageCtx_ The current GraphicPassStageContext.
         */
        virtual void before(const ptr<const RenderContext> ctx_, cref<GraphicPassStageContext> stageCtx_) = 0;

        /**
         * Process the given batch_
         *
         * @author Julius
         * @date 17.01.2021
         *
         * @param  ctx_ The RenderContext containing the resources, scene, camera and target data.
         * @param  stageCtx_ The current GraphicPassStageContext.
         * @param  batch_ The batch.
         */
        virtual void process(const ptr<const RenderContext> ctx_, cref<GraphicPassStageContext> stageCtx_,
            ptr<const ProcessedModelBatch> batch_) = 0;

        /**
         * Postprocess for Batches.
         * 
         * @ note Will be invoked once per GraphicPassPipeline invocation if any check for positive
         *
         * @author Julius
         * @date 12.02.2021
         *
         * @param  ctx_ The RenderContext containing the resources, scene, camera and target data.
         * @param  stageCtx_ The current GraphicPassStageContext.
         */
        virtual void after(const ptr<const RenderContext> ctx_, cref<GraphicPassStageContext> stageCtx_) = 0;
    };
}
