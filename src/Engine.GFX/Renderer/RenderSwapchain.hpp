#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/Collection/RingBuffer.hpp>

#include "RenderInvocationState.hpp"

namespace ember::engine::gfx {

    class RenderSwapchain {
    public:
        using this_type = RenderSwapchain;

    public:
        RenderSwapchain() = delete;

        RenderSwapchain(const u8 size_);

        RenderSwapchain(cref<this_type>) = delete;

        RenderSwapchain(mref<this_type>) = delete;

        ~RenderSwapchain();

    private:
        _STD atomic_uint8_t _desired;

        concurrent::RingBuffer<u8> _swaps;

    private:
        void init(const u8 size_);

    public:
        [[nodiscard]] u8 desiredSize() const noexcept;

    public:
        [[nodiscard]] bool acquireNextFrame(_Inout_ ref<RenderInvocationState> state_);
    };
}
