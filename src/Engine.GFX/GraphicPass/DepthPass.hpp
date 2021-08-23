#pragma once
#include "DepthModelPassProcessor.hpp"
#include "../GraphicPass.hpp"
#include "../Framebuffer/Framebuffer.hpp"

namespace ember::engine::gfx {
    class DepthPass :
        public GraphicPass {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 30.01.2021
         */
        DepthPass();

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
        /**
         *
         */
        DepthModelPassProcessor _processor;

    public:
        /**
         * Creates a ModelPassProcessor
         *
         * @author Julius
         * @date 22.01.2021
         *
         * @returns A ModelPassProcessor.
         */
        ptr<ModelPassProcessor> processor() noexcept override final;

    private:
        /**
         * Framebuffers
         */
        vector<Framebuffer> _framebuffers;

    public:
        /**
         * Get Framebuffer at the given index
         *
         * @author Julius
         * @date 31.01.2021
         *
         * @param  idx_ Zero-based index of the.
         *
         * @returns A cref&lt;Framebuffer&gt;
         */
        [[nodiscard]] cref<Framebuffer> framebuffer(u32 idx_) const;

        /**
         * Get the current Framebuffer
         *
         *  Substituted from Swapchain and Swapchain Idx
         *
         * @author Julius
         * @date 31.01.2021
         *
         * @returns A cref&lt;Framebuffer&gt;
         */
        [[nodiscard]] cref<Framebuffer> currentFramebuffer() const noexcept;
    };
}
