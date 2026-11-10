#pragma once
#include <Engine.Common/Types.hpp>

namespace hg::engine::gfx {
    enum class PrimitiveTopology : u8 {
        eTriangleList = 0x21,
        ePatchList = 0x31
    };
}
