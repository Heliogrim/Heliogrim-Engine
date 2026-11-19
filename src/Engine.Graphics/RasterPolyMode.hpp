#pragma once
#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
    enum class RasterPolyMode : u8 {
        eFace = 0x0,
        eLine = 0x1
    };
}
