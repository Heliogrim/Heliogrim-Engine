#pragma once

#include <Engine.Accel.Lang/Rep/Symbol.hpp>
#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::accel {
    struct StageOutput final {
        nmpt<const lang::Symbol> symbol;
    };
}
