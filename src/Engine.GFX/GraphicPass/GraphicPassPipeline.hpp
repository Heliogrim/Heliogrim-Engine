#pragma once
#include <Engine.Common/Collection/Vector.hpp>

#include "GraphicPassPipelineStage.hpp"
#include "GraphicPassModelProcessor.hpp"
#include "../Renderer/RenderContext.hpp"

namespace ember::engine::gfx {

    class GraphicPassPipeline {
    public:
        /**
         * Forwards allocation call with invocation data and state where to allocate
         *
         * @author Julius
         * @date 04.03.2021
         *
         * @param invocation_ The RenderInvocation data which is required for the allocation
         * @param state_ The RenderInvocationState where to store the allocated data
         */
        void allocateWith(_In_ const ptr<const RenderPass> invocation_,
            _Inout_ const ptr<RenderPassState> state_);

        /**
         * Forwards free call with invocation data and state where to free from
         *
         * @author Julius
         * @date 04.03.2021
         *
         * @details Will invoke free calls in reverse order related to allocation
         *
         * @param invocation_ The RenderInvocation data which was required for the allocation
         * @param state_ The RenderInvocationState where to free data from
         */
        void freeWith(_In_ const ptr<const RenderPass> invocation_,
            _Inout_ const ptr<RenderPassState> state_);

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
         * @param  ctx_ The RenderContext containing the resources, scene, camera and target data.
         * @param batch_ The recording command batch.
         */
        void process(ptr<const GraphicPassModelProcessor> processor_, _In_ const ptr<const RenderContext> ctx_,
            _Inout_ ref<CommandBatch> batch_);

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
