#pragma once
#include "../Inc.hpp"
#include "../Library/Device.hpp"
#include "../Library/Swapchain.hpp"

namespace clockwork::__internal::gfx::handle {
    class LoopHandler {
    public:

    private:
        /**
         * Device
         */
        clockwork::__internal::gfx::types::__Device _device;

    private:
        /**
         * Swapchain
         */
        clockwork::__internal::gfx::types::__Swapchain _swapchain;

    private:
        /**
         * Mutation
         */

    public:
        void invoke() {
            uint32_t nextImageIndex;
            vk::Result result = _device->logDev.acquireNextImageKHR(
                *_swapchain,
                UINT64_MAX,
                _semaphore,
                nullptr,
                &nextImageIndex
            )

            /**
             * Check Next Image Result
             */
            if (result == vk::Result::eErrorOutOfDateKHR) {
                // TODO: recover swapchain
            }
        }

        void operator()() {
            invoke();
        }
    };
}
