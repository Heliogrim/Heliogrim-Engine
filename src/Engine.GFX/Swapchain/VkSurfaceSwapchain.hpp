#pragma once

#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "Swapchain.hpp"

namespace hg::engine::gfx {
    class VkSurfaceSwapchain final :
        public Swapchain {
    public:
        using this_type = VkSurfaceSwapchain;
        using underlying_type = Swapchain;

    public:
        VkSurfaceSwapchain(nmpt<Surface> owner_);

        ~VkSurfaceSwapchain() override;

    public:
        void setup(cref<sptr<Device>> device_) override;

        void destroy() override;

    public:
        [[nodiscard]] s64 imageCount() const noexcept override;

    public:
        bool acquireNext(ref<s64> idx_, ref<smr<Texture>> image_, ref<vk::Semaphore> signal_) override;

        vk::Result presentNext(u64 idx_) override;

        vk::Result presentNext(u64 idx_, cref<Vector<vk::Semaphore>> waits_) override;

    public:
        bool consumeNext(
            _Out_ ref<smr<Texture>> image_,
            _Out_ ref<vk::Semaphore> signal_,
            _Out_ ref<Vector<vk::Semaphore>> waits_
        ) override;

    private:
        Vector<vk::Semaphore> _signals;
        vk::SwapchainKHR _vkSwapchain;

        nmpt<Surface> _surface;

    private:
        [[nodiscard]] vk::Semaphore nextSignal();

        void restoreSignal(vk::Semaphore signal_);
    };
}
