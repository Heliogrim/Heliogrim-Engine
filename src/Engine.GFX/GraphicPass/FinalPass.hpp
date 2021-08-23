#pragma once
#include "../GraphicPass.hpp"
#include "../Framebuffer/Framebuffer.hpp"

namespace ember::engine::gfx {
    class FinalPass final :
        public GraphicPass {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 30.01.2021
         */
        FinalPass();

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

        [[nodiscard]] void process([[maybe_unused]] cref<scene::SceneGraph> graph_, ref<CommandBatch> batch_) override;

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
