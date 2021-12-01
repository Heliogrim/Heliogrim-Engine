#pragma once

#include "DepthPass.hpp"
#include "../GraphicPass.hpp"
#include "../Framebuffer/Framebuffer.hpp"

namespace ember::engine::gfx {
    class PbrPass :
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
        PbrPass(cref<sptr<Device>> device_, const ptr<Swapchain> swapchain_, const ptr<DepthPass> depthPass_);

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

        /**
         * Creates a ModelPassProcessor
         *
         * @author Julius
         * @date 22.01.2021
         *
         * @returns A ModelPassProcessor.
         */
        ptr<ModelPassProcessor> processor() noexcept override;

    private:
        ptr<DepthPass> _depthPass;

    private:
        /**
         * Framebuffer
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
