#pragma once
#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
    enum class InputRate : u8 {
        ePerVertex = 0x0,
        ePerInstance = 0x1
    };
}
