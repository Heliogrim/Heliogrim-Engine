#pragma once
#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
    enum class RasterCullFace : u8 {
        eNone = 0x0,
        eBack = 0x1,
        eFront = 0x2,
        eBackAndFront = 0x3
    };
}
