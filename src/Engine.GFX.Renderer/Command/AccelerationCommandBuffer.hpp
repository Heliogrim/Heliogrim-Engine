#pragma once

#include <Engine.GFX/Pipeline/LORenderPass.hpp>

namespace hg::engine::gfx::render {
    class AccelerationCommandBuffer {
    public:
        [[nodiscard]] bool isBatchingBuffer() const noexcept;

        [[nodiscard]] bool isForwardBuffer() const noexcept;

    public:
        void beginRenderPass(cref<pipeline::LORenderPass> loRenderPass_, cref<Framebuffer> framebuffer_);
    };
}
