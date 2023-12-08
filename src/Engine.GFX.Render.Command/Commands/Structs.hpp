#pragma once

#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX/Framebuffer/Framebuffer.hpp>

namespace hg::engine::render::cmd {
    struct BeginAccelerationPassStruct {
        nmpt<const accel::AccelerationPass> pass;
        nmpt<const gfx::Framebuffer> framebuffer;
    };

    struct BeginSubPassStruct {
        nmpt<const gfx::Framebuffer> framebuffer = nullptr;
    };
}
