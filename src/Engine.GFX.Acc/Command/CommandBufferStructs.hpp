#pragma once
#include <Engine.GFX/Framebuffer/Framebuffer.hpp>

#include "../Pass/GraphicsPass.hpp"

namespace hg::engine::gfx::acc {
    struct BindRenderPassStruct final {
        cref<GraphicsPass> graphicsPass;
        cref<Framebuffer> framebuffer;
        bool subrecording = false;
    };
}
