#pragma once
#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
    enum class RasterPolyFront: u8 {
        eCounterClockwise = 0x0,
        eClockwise = 0x1
    };
}
