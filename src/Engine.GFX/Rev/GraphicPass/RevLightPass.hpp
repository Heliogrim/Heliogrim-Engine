#pragma once
#include "../../GraphicPass/GraphicPass.hpp"

namespace ember::engine::gfx {
    class RevLightPass :
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
        RevLightPass(cref<sptr<Device>> device_);

        /**
         * Setups this 
         *
         * @author Julius
         * @date 22.01.2021
         */
        virtual void setup() override;

        /**
         * Destroys this 
         *
         * @author Julius
         * @date 22.01.2021
         */
        virtual void destroy() override;

        /**
         * Creates a GraphicPassModelProcessor
         *
         * @author Julius
         * @date 22.01.2021
         *
         * @returns A GraphicPassModelProcessor.
         */
        virtual ptr<GraphicPassModelProcessor> processor() noexcept override;
    };
}
