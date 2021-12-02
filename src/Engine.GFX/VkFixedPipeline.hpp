#pragma once
#include "FixedPipeline.hpp"
#include "Pipeline/RenderPass.hpp"

namespace ember::engine::gfx {
    class VkFixedPipeline final :
        public FixedPipeline {
    public:

        /**
         * Constructor
         *
         * @author Julius
         * @date 25.01.2021
         *
         * @param  device_ The device.
         * @param  renderPass_ The render pass.
         */
        VkFixedPipeline(sptr<Device> device_, sptr<pipeline::RenderPass> renderPass_);

        /**
         * Destructor
         *
         * @author Julius
         * @date 25.01.2021
         */
        ~VkFixedPipeline() noexcept override;

        /**
         * Setups this 
         *
         * @author Julius
         * @date 25.01.2021
         */
        void setup() override;

        /**
         * Destroys this 
         *
         * @author Julius
         * @date 25.01.2021
         */
        void destroy() noexcept override;

    private:
        /**
         * Device
         */
        sptr<Device> _device;

    private:
        /**
         * RenderPass
         */
        sptr<pipeline::RenderPass> _renderPass;

    private:
        /**
         * Blend Post-Process
         */
        Vector<vk::PipelineColorBlendAttachmentState> _blending;

    public:

        /**
         * Gets the ColorBlendState
         *
         * @author Julius
         * @date 08.02.2021
         *
         * @returns A ref&lt;vector&lt;vk::PipelineColorBlendAttachmentState&gt;&gt;
         */
        [[nodiscard]] ref<Vector<vk::PipelineColorBlendAttachmentState>> blending() noexcept;

    private:
        /**
         * Layout
         */
        vk::PipelineLayout _layout;

    public:
        /**
         * Vk layout
         *
         * @author Julius
         * @date 25.01.2021
         *
         * @returns A cref&lt;vk::PipelineLayout&gt;
         */
        [[nodiscard]] cref<vk::PipelineLayout> vkLayout() const noexcept;

    private:
        /**
         * Pipeline
         */
        vk::Pipeline _pipeline;

    public:
        /**
         * Vk pipeline
         *
         * @author Julius
         * @date 25.01.2021
         *
         * @returns A cref&lt;vk::Pipeline&gt;
         */
        [[nodiscard]] cref<vk::Pipeline> vkPipeline() const noexcept;
    };
}
