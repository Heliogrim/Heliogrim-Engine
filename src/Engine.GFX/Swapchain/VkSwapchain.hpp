#pragma once
#include "Swapchain.hpp"

namespace ember::engine::gfx {
    class VkSwapchain :
        public Swapchain {
    public:

        /**
         * Constructor
         *
         * @author Julius
         * @date 30.01.2021
         *
         * @param  device_ The device.
         * @param  surface_ The surface.
         */
        VkSwapchain(cref<sptr<Device>> device_, cref<Surface> surface_);

        /**
         * Destructor
         *
         * @author Julius
         * @date 30.01.2021
         */
        ~VkSwapchain() override;

        /**
         * Setups this 
         *
         * @author Julius
         * @date 30.01.2021
         */
        virtual void setup() override;

        /**
         * Destroys this 
         *
         * @author Julius
         * @date 30.01.2021
         */
        virtual void destroy() override;

    private:
        /**
         * Swapchain Instance
         */
        vk::SwapchainKHR _vkSwapchain;

    public:
        /**
         * Vk swapchain
         *
         * @author Julius
         * @date 30.01.2021
         *
         * @returns A cref&lt;vk::Swapchain&gt;
         */
        [[nodiscard]] cref<vk::SwapchainKHR> vkSwapchain() const noexcept;

    private:
        /**
         * Device
         */
        sptr<Device> _device;

    private:
        /**
         * Surface
         */
        Surface _surface;

    private:
        /**
         * Clamp extent
         *
         * @author Julius
         * @date 30.01.2021
         *
         * @param  extent_ The extent.
         * @param  capabilities_ The capabilities.
         *
         * @returns A math::uivec2.
         */
        [[nodiscard]] math::uivec2 clampExtent(math::uivec2 extent_,
            cref<vk::SurfaceCapabilitiesKHR> capabilities_) noexcept;

        /**
         * Select present mode
         *
         * @author Julius
         * @date 30.01.2021
         *
         * @param  modes_ The modes.
         *
         * @returns A vk::PresentModeKHR.
         */
        [[nodiscard]] vk::PresentModeKHR selectPresentMode(cref<vector<vk::PresentModeKHR>> modes_) const noexcept;
    };
}
