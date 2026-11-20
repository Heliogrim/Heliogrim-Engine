#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow {
    enum class ReflowWrap: u8 {
        eNone = 0u,
        eNoWrap = 0u,
        eWrap = 1u
    };
}
