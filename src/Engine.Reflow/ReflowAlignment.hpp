#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow {
    enum class ReflowAlignment : u8 {
        eStart = 0u,
        eCenter = 1u,
        eEnd = 2u
    };
}
