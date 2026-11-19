#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow {
    enum class EventResponse : u8 {
        eUnhandled = 0x0,
        eHandled = 0x1,
        //
        eConsumed = eHandled | 0x2
    };
}
