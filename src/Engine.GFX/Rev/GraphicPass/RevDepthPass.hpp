#pragma once
#include "RevDepthPassModelProcessor.hpp"
#include "../../GraphicPass/GraphicPass.hpp"
#include "../../Framebuffer/Framebuffer.hpp"

namespace ember::engine::gfx {

    class RevDepthPass :
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
        RevDepthPass(cref<sptr<Device>> device_);

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
        void postProcessAllocated(const ptr<RenderInvocationState> state_) const;

    public:
        void allocateWith(const ptr<const RenderInvocation> invocation_,
            const ptr<RenderInvocationState> state_) override;

        void freeWith(const ptr<const RenderInvocation> invocation_, const ptr<RenderInvocationState> state_) override;

    private:
        /**
         *
         */
        RevDepthPassModelProcessor _processor;

    public:
        /**
         * Creates a GraphicPassModelProcessor
         *
         * @author Julius
         * @date 22.01.2021
         *
         * @returns A GraphicPassModelProcessor.
         */
        ptr<GraphicPassModelProcessor> processor() noexcept override final;
    };
}
