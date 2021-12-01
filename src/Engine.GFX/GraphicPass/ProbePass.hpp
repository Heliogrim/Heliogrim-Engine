#pragma once

#include "../GraphicPass.hpp"

namespace ember::engine::gfx {
    class ProbePass :
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
        ProbePass(cref<sptr<Device>> device_, const ptr<Swapchain> swapchain_);

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
         * Creates a ModelPassProcessor
         *
         * @author Julius
         * @date 22.01.2021
         *
         * @returns A ModelPassProcessor.
         */
        virtual ptr<ModelPassProcessor> processor() noexcept override;
    };
}
