#pragma once
#include "../FixedPipeline.hpp"
#include "../GraphicPassPipelineStage.hpp"
#include "../Pipeline/RenderPass.hpp"

namespace ember::engine::gfx {

    /**
     * Forward Declaration
     */
    class DepthPass;

    class DepthPassSkeletalStage final :
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
        DepthPassSkeletalStage(ptr<DepthPass> graphicPass_);

        /**
         * Setups this 
         *
         * @author Julius
         * @date 08.02.2021
         */
        void setup() override;

        /**
         * Destroys this 
         *
         * @author Julius
         * @date 08.02.2021
         */
        void destroy() noexcept override;

        /**
         * Checks whether the given batch should be processed in this stage
         *
         * @author Julius
         * @date 08.02.2021
         *
         * @param  batch_ The batch.
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool check(ptr<const ProcessedModelBatch> batch_) noexcept override;

        void before(cref<GraphicPassStageContext> ctx_) override;

        /**
         * Process the given batch_
         *
         * @author Julius
         * @date 08.02.2021
         *
         * @param  ctx_ The context.
         * @param  batch_ The batch.
         */
        void process(cref<GraphicPassStageContext> ctx_, ptr<const ProcessedModelBatch> batch_) override;

        void after(cref<GraphicPassStageContext> ctx_);

    private:
        /**
         * Graphic Pass
         */
        ptr<DepthPass> _graphicPass;

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

    public:

        /**
         * Renders the pass
         *
         * @author Julius
         * @date 08.02.2021
         *
         * @returns A sptr&lt;pipeline::RenderPass&gt;
         */
        [[nodiscard]] sptr<pipeline::RenderPass> renderPass() const noexcept;

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
         * @date 08.02.2021
         *
         * @returns A sptr&lt;FixedPipeline&gt;
         */
        [[nodiscard]] sptr<FixedPipeline> pipeline() const noexcept;
    };
}
