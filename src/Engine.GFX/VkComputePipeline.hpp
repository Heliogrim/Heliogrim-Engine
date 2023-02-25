#pragma once
#include "ComputePipeline.hpp"
#include "Device/Device.hpp"

namespace ember::engine::gfx {
    class VkComputePipeline final :
        public ComputePipeline {
    public:
        VkComputePipeline(
            sptr<Device> device_
        );

        ~VkComputePipeline() noexcept override;

    public:
        void setup() override;

        void destroy() noexcept override;

    private:
        sptr<Device> _device;

    private:
        vk::PipelineLayout _vkLayout;

    public:
        [[nodiscard]] cref<vk::PipelineLayout> vkLayout() const noexcept;

    private:
        vk::Pipeline _vkPipeline;

    public:
        [[nodiscard]] cref<vk::Pipeline> vkPipeline() const noexcept;
    };
}
