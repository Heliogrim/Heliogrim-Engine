#pragma once

#include "Swapchain.hpp"

namespace ember::engine::gfx {

    class VkSurfaceSwapchain final :
        public Swapchain {
    public:
        using this_type = VkSurfaceSwapchain;
        using underlying_type = Swapchain;

    public:
        VkSurfaceSwapchain();

        ~VkSurfaceSwapchain() override;

    public:
        void setup(cref<sptr<Device>> device_) override;

        void destroy() override;

    public:
        bool acquireNext(ref<s64> idx_, ref<sptr<Texture>> image_, ref<vk::Semaphore> signal_) override;

        vk::Result presentNext(u64 idx_) override;

        vk::Result presentNext(u64 idx_, cref<Vector<vk::Semaphore>> waits_) override;

    public:
        bool consumeNext(ref<Texture> image_, ref<Vector<vk::Semaphore>> waits_) override;

    private:
        Vector<vk::Semaphore> _signals;
        vk::SwapchainKHR _vkSwapchain;

        Surface _surface;

    private:
        [[nodiscard]] vk::Semaphore nextSignal();

        void restoreSignal(vk::Semaphore signal_);

    public:
        void useSurface(cref<Surface> surface_);
    };

}
