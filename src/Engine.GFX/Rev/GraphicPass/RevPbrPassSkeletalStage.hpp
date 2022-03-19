#pragma once
#include "../../GraphicPass/GraphicPassPipelineStage.hpp"
#include "../../FixedPipeline.hpp"
#include "../../Pipeline/RenderPass.hpp"

namespace ember::engine::gfx {

    /**
     * Forward Declaration
     */
    class RevPbrPass;

    class RevPbrPassSkeletalStage final :
        public GraphicPassPipelineStage {
    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 12.02.2021
         *
         * @param  graphicPass_ The graphic pass.
         */
        RevPbrPassSkeletalStage(ptr<RevPbrPass> graphicPass_);

        /**
         * Setups this 
         *
         * @author Julius
         * @date 25.01.2021
         */
        void setup() override;

        /**
         * Destroys this 
         *
         * @author Julius
         * @date 25.01.2021
         */
        void destroy() noexcept override;

    public:
        void allocateWith(const ptr<const RenderInvocation> invocation_,
            const ptr<RenderInvocationState> state_) override;

        void freeWith(const ptr<const RenderInvocation> invocation_, const ptr<RenderInvocationState> state_) override;

    public:
        /**
         * Checks whether the given batch should be processed in this stage
         *
         * @author Julius
         * @date 25.01.2021
         *
         * @param  batch_ The batch.
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool check(ptr<const ProcessedModelBatch> batch_) noexcept override;

        void before(const ptr<const RenderContext> ctx_, cref<GraphicPassStageContext> stageCtx_) override;

        /**
         * Process the given batch_
         *
         * @author Julius
         * @date 25.01.2021
         *
         * @param  ctx_ The RenderContext containing the resources, scene, camera and target data.
         * @param  stageCtx_ The current GraphicPassStageContext.
         * @param  batch_ The batch.
         */
        void process(const ptr<const RenderContext> ctx_, cref<GraphicPassStageContext> stageCtx_,
            ptr<const ProcessedModelBatch> batch_) override;

        void after(const ptr<const RenderContext> ctx_, cref<GraphicPassStageContext> stageCtx_) override;

    private:
        /**
         * Graphic Pass
         */
        ptr<RevPbrPass> _graphicPass;

    private:
        /**
         * Command Buffer
         */
        sptr<CommandBuffer> _cmd;

    private:
        /**
         * Render Pass
         */
        sptr<pipeline::RenderPass> _renderPass;

    private:
        /**
         * Graphics Pipeline
         */
        sptr<FixedPipeline> _pipeline;

    public:
        /**
         * Gets the internal Graphics Pipeline
         *
         * @author Julius
         * @date 25.01.2021
         *
         * @returns A sptr&lt;FixedPipeline&gt;
         */
        [[nodiscard]] sptr<FixedPipeline> pipeline() const noexcept;
    };
}
