#pragma once

namespace hg::engine::gfx::render::cmd {
    class NativeQueue {
    public:
        using this_type = NativeQueue;

    public:
        constexpr NativeQueue() noexcept = default;

        constexpr ~NativeQueue() noexcept = default;
    };
}
