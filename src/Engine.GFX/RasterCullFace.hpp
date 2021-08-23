#pragma once
#include <Engine.Common/Types.hpp>

namespace ember::engine::gfx {
    enum class RasterCullFace : u8 {
        eNone = 0x0,
        eBack = 0x1,
        eFront = 0x2,
        eBackAndFront = 0x3
    };
}
