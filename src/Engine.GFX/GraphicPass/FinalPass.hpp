#pragma once
#include "../GraphicPass.hpp"
#include "../Framebuffer/Framebuffer.hpp"

namespace ember::engine::gfx {
    class FinalPass final :
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
        FinalPass(cref<sptr<Device>> device_, const ptr<Swapchain> swapchain_);

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

        void process([[maybe_unused]] const ptr<scene::SceneGraph> graph_,
            ref<CommandBatch> batch_) override;

        /**
         * Creates a ModelPassProcessor
         *
         * @author Julius
         * @date 17.01.2021
         *
         * @returns A ModelPassProcessor.
         */
        [[nodiscard]] ptr<ModelPassProcessor> processor() noexcept override;

    private:
        /**
         * Framebuffer
         */
        Vector<Framebuffer> _framebuffers;

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
