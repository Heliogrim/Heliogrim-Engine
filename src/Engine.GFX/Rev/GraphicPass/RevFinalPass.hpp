#pragma once
#include "../../GraphicPass/GraphicPass.hpp"

namespace ember::engine::gfx {

    class RevFinalPass final :
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
        RevFinalPass(cref<sptr<Device>> device_);

        /**
         * Setups this 
         *
         * @author Julius
         * @date 17.01.2021
         */
        void setup() override;

        /**
         * Destroys this 
         *
         * @author Julius
         * @date 17.01.2021
         */
        void destroy() override;

    public:
        void allocateWith(const ptr<const RenderPass> invocation_,
            const ptr<RenderPassState> state_) override;

        void freeWith(const ptr<const RenderPass> invocation_, const ptr<RenderPassState> state_) override;

        void process([[maybe_unused]] _In_ const ptr<scene::RenderGraph> graph_,
            _In_ const ptr<const RenderContext> ctx_,
            _Inout_ ref<CommandBatch> batch_) override;

        /**
         * Creates a GraphicPassModelProcessor
         *
         * @author Julius
         * @date 17.01.2021
         *
         * @returns A GraphicPassModelProcessor.
         */
        [[nodiscard]] ptr<GraphicPassModelProcessor> processor() noexcept override;
    };
}
