#pragma once
#include <Engine.Accel.Pass/GraphicsPass.hpp>
#include <Engine.GFX/Framebuffer/Framebuffer.hpp>

namespace hg::engine::accel {
    struct BindRenderPassStruct final {
        cref<GraphicsPass> graphicsPass;
        cref<gfx::Framebuffer> framebuffer;
        bool subrecording = false;
    };
}
