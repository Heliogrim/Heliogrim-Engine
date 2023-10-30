#pragma once
#include <Engine.GFX/Command/CommandBuffer.hpp>

namespace hg::engine::gfx::acc {
    //class CommandBuffer { };
    using CommandBuffer = ::hg::engine::gfx::CommandBuffer;
}

namespace hg::engine::gfx {
    using AccelerationCommandBuffer = ::hg::engine::gfx::acc::CommandBuffer;
}
