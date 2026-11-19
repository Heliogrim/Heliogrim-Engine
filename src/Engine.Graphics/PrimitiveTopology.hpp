#pragma once
#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
    enum class PrimitiveTopology : u8 {
        eTriangleList = 0x21,
        ePatchList = 0x31
    };
}
