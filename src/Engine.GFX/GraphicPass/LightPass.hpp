#pragma once
#include "../GraphicPass.hpp"

namespace ember::engine::gfx {
    class LightPass :
        public GraphicPass {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 30.01.2021
         */
        LightPass();

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
