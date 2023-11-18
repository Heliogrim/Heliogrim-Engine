#pragma once

#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX/Framebuffer/Framebuffer.hpp>
#include <Engine.GFX.Acc/AccelerationPass.hpp>

namespace hg::engine::gfx::render::cmd {
    struct BeginAccelerationPassStruct {
        nmpt<const acc::AccelerationPass> pass;
        nmpt<const Framebuffer> framebuffer;
    };

    struct BeginSubPassStruct {
        nmpt<const Framebuffer> framebuffer = nullptr;
    };
}
