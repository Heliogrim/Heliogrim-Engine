#pragma once
#include <Engine.Common/Types.hpp>

namespace hg::engine::gfx::render::graph {
    enum class LinkMode : u8 {
        eInclusive = 0x0,
        eExclusive = 0x1
    };
}
