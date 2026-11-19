#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
    enum class RenderEnqueueResult : u8 {
        eSuccess = 0x0,
        eFailed = 0x1,
        //
        eFailedChanged = 0x2
    };
}
