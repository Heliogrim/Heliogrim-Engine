#pragma once

#include "RevDepthPass.hpp"
#include "RevPbrPassModelProcessor.hpp"
#include "../../GraphicPass/GraphicPass.hpp"

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
         * @see GraphicPass::GraphicPass(...)
         */
        RevPbrPass(cref<sptr<Device>> device_);

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
        RevPbrPassModelProcessor _processor;

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
