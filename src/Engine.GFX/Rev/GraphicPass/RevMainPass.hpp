#pragma once

#include "RevDepthPass.hpp"
#include "RevMainPassModelProcessor.hpp"
#include "../../GraphicPass/GraphicPass.hpp"

namespace ember::engine::gfx {

    class RevMainPass :
        public GraphicPass {
    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 27.11.2021
         *
         * @see GraphicPass::GraphicPass(...)
         */
        RevMainPass(cref<sptr<Device>> device_);

    private:
        sptr<pipeline::ApiRenderPass> _renderPass;

    public:
        [[nodiscard]] cref<sptr<pipeline::ApiRenderPass>> renderPass() const noexcept;

    private:
        void setupRenderPass();

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
        void destroy() override;

    private:
        void postProcessAllocated(const ptr<RenderPassState> state_) const;

    public:
        void allocateWith(const ptr<const RenderPass> invocation_,
            const ptr<RenderPassState> state_) override;

        void freeWith(const ptr<const RenderPass> invocation_, const ptr<RenderPassState> state_) override;

    public:
        void process(const ptr<scene::RenderGraph> graph_, const ptr<const RenderContext> ctx_,
            ref<CommandBatch> batch_) override;

    private:
        /**
         *
         */
        RevMainPassModelProcessor _processor;

    public:
        /**
         * Creates a GraphicPassModelProcessor
         *
         * @author Julius
         * @date 22.01.2021
         *
         * @returns A GraphicPassModelProcessor.
         */
        ptr<GraphicPassModelProcessor> processor() noexcept override;
    };
}
