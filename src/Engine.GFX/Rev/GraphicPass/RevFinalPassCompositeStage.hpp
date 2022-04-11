#pragma once

#if FALSE

#include "../../FixedPipeline.hpp"
#include "../../GraphicPass/GraphicPassPipelineStage.hpp"
#include "../../Pipeline/ApiRenderPass.hpp"
#include "../../Shader/DiscreteBindingGroup.hpp"

namespace ember::engine::gfx {

    /**
     * Forward Declaration
     */
    class RevFinalPass;

    class RevFinalPassCompositeStage final :
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
        RevFinalPassCompositeStage(ptr<RevFinalPass> graphicPass_);

    private:
        void setupShader();

    public:
        /**
         * Setups this 
         *
         * @author Julius
         * @date 22.01.2021
         */
        void setup() override;

        /**
         * Destroys this 
         *
         * @author Julius
         * @date 22.01.2021
         */
        void destroy() noexcept override;

    public:
        void allocateWith(const ptr<const RenderPass> invocation_,
            const ptr<RenderPassState> state_) override;

        void freeWith(const ptr<const RenderPass> invocation_, const ptr<RenderPassState> state_) override;

    public:
        /**
         * Checks whether the given batch should be processed in this stage
         *
         * @author Julius
         * @date 22.01.2021
         *
         * @param  batch_ The batch.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool check(ptr<const ProcessedModelBatch> batch_) noexcept override;

        void before(const ptr<const RenderContext> ctx_, cref<GraphicPassStageContext> stageCtx_) override;

        /**
         * Process the given batch_
         *
         * @author Julius
         * @date 22.01.2021
         *
         * @param  ctx_ The RenderContext containing the resources, scene, camera and target data.
         * @param  stageCtx_ The current GraphicPassStageContext.
         * @param  batch_ The batch.
         */
        void process(const ptr<const RenderContext> ctx_, cref<GraphicPassStageContext> stageCtx_, ptr<const ProcessedModelBatch> batch_) override;

        void after(const ptr<const RenderContext> ctx_, cref<GraphicPassStageContext> stageCtx_) override;

    private:
        /**
         * Graphic Pass
         */
        ptr<RevFinalPass> _graphicPass;

    private:
        /**
         * Render Pass
         */
        sptr<pipeline::ApiRenderPass> _renderPass;

    public:
        /**
         * Renders the pass
         *
         * @author Julius
         * @date 05.02.2021
         *
         * @returns A sptr&lt;pipeline::RenderPass&gt;
         */
        [[nodiscard]] sptr<pipeline::ApiRenderPass> renderPass() const noexcept;

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
         * @date 22.01.2021
         *
         * @returns A sptr&lt;FixedPipeline&gt;
         */
        [[nodiscard]] sptr<FixedPipeline> pipeline() const noexcept;

    private:
        Vector<vk::DescriptorPoolCreateInfo> _requiredDescriptorPools;
        Vector<shader::ShaderBindingGroup> _requiredBindingGroups;
    };
}


#endif
