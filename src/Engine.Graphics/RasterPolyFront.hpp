#pragma once
#include <Engine.Common/Types.hpp>

namespace hg::engine::graphics {
    enum class RasterPolyFront: u8 {
        eCounterClockwise = 0x0,
        eClockwise = 0x1
    };
}
