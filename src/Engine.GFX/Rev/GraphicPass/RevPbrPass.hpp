#pragma once

#include "RevDepthPass.hpp"
#include "../../GraphicPass/GraphicPass.hpp"
#include "../../Framebuffer/Framebuffer.hpp"

namespace ember::engine::gfx {
    class RevPbrPass :
        public GraphicPass {
    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 27.11.2021
         *
         * @param depthPass_ The previous depth pass to use
         *
         * @see GraphicPass::GraphicPass(...)
         */
        RevPbrPass(cref<sptr<Device>> device_, const ptr<RevDepthPass> depthPass_);

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

    public:
        void allocateWith(const ptr<const RenderInvocation> invocation_,
            const ptr<RenderInvocationState> state_) override;

        void freeWith(const ptr<const RenderInvocation> invocation_, const ptr<RenderInvocationState> state_) override;

        /**
         * Creates a GraphicPassModelProcessor
         *
         * @author Julius
         * @date 22.01.2021
         *
         * @returns A GraphicPassModelProcessor.
         */
        ptr<GraphicPassModelProcessor> processor() noexcept override;

    private:
        ptr<RevDepthPass> _depthPass;
    };
}
