#pragma once
#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
    enum class RasterSamples : u8 {
        e1 = 0x1 << 0,
        e4 = 0x1 << 1,
        e8 = 0x1 << 2,
        e16 = 0x1 << 3,
        e32 = 0x1 << 4,
        e64 = 0x1 << 5
    };
}
