#pragma once
#include "../../FixedPipeline.hpp"
#include "../../GraphicPass/GraphicPassPipelineStage.hpp"
#include "../../Pipeline/RenderPass.hpp"
#include "../../Shader/DiscreteBindingGroup.hpp"

namespace ember::engine::gfx {
    /**
     * Forward Declaration
     */
    class RevDepthPass;

    class RevDepthPassStaticStage final :
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
        RevDepthPassStaticStage(const ptr<RevDepthPass> graphicPass_);

    private:
        void setupShader();

    public:
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
        bool check(ptr<const ProcessedModelBatch> batch_) noexcept override;

        void before(cref<GraphicPassStageContext> ctx_) override;

        /**
         * Process the given batch_
         *
         * @author Julius
         * @date 25.01.2021
         *
         * @param  ctx_ The context.
         * @param  batch_ The batch.
         */
        void process(cref<GraphicPassStageContext> ctx_, ptr<const ProcessedModelBatch> batch_) override;

        void after(cref<GraphicPassStageContext> ctx_) override;

    private:
        /**
         * Graphic Pass
         */
        ptr<RevDepthPass> _graphicPass;

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
         * @date 04.02.2021
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
         * @date 25.01.2021
         *
         * @returns A sptr&lt;FixedPipeline&gt;
         */
        [[nodiscard]] sptr<FixedPipeline> pipeline() const noexcept;

    private:
        Vector<vk::DescriptorPoolCreateInfo> _requiredDescriptorPools;
        Vector<shader::ShaderBindingGroup> _requiredBindingGroups;
    };
}
