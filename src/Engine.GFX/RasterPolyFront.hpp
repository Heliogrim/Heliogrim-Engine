#pragma once
#include <Engine.Common/Types.hpp>

namespace ember::engine::gfx {
    enum class RasterPolyFront: u8 {
        eCounterClockwise = 0x0,
        eClockwise = 0x1
    };
}
