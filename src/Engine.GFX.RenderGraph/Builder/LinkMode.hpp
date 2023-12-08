#pragma once
#include <Engine.Common/Types.hpp>

namespace hg::engine::render::graph {
    enum class LinkMode : u8 {
        eInclusive = 0x0,
        eExclusive = 0x1
    };
}
