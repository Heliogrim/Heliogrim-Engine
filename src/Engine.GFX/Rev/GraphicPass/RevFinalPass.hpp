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
        void allocateWith(const ptr<const RenderInvocation> invocation_,
            const ptr<RenderInvocationState> state_) override;

        void freeWith(const ptr<const RenderInvocation> invocation_, const ptr<RenderInvocationState> state_) override;

        void process([[maybe_unused]] _In_ const ptr<scene::RenderGraph> graph_,
            _Inout_ ref<RenderInvocationState> state_,
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
