#pragma once

#if FALSE

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
        void postProcessAllocated(const ptr<RenderPassState> state_) const;

    public:
        void allocateWith(const ptr<const HORenderPass> invocation_,
            const ptr<RenderPassState> state_) override;

        void freeWith(const ptr<const HORenderPass> invocation_, const ptr<RenderPassState> state_) override;

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

#endif
