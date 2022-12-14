#pragma once

#include "Swapchain.hpp"

#include <Engine.Common/Concurrent/Collection/RingBuffer.hpp>

namespace ember::engine::gfx {
    class VkSwapchain final :
        public Swapchain {
    public:
        using this_type = VkSwapchain;
        using underlying_type = Swapchain;

    public:
        VkSwapchain();

        ~VkSwapchain() override;

    public:
        void setup(cref<sptr<Device>> device_) override;

        void destroy() override;

    private:
        concurrent::RingBuffer<u64> _rQueue;
        concurrent::RingBuffer<u64> _pQueue;

    public:
        bool acquireNext(ref<s64> idx_, ref<sptr<Texture>> image_, ref<vk::Semaphore> signal_) override;

        vk::Result presentNext(u64 idx_) override;

        vk::Result presentNext(u64 idx_, cref<Vector<vk::Semaphore>> waits_) override;

    public:
        bool consumeNext(_Out_ ref<sptr<Texture>> image_, _Out_ ref<vk::Semaphore> signal_,
            _Out_ ref<Vector<vk::Semaphore>> waits_) override;

    public:
        void setExtent(cref<math::uivec2> extent_);

        void setFormat(TextureFormat format_);

    private:
        u32 _desiredImages;

    public:
        void setDesiredImages(const u32 desired_);

    private:
        [[nodiscard]] vk::Semaphore nextSignal();
    };
}
