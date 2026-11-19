#pragma once

#include <Engine.Common/__macro.hpp>

#include "__fwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::driver::vk {
    template <typename AccelerationPassType_>
    struct SubpassInfo {
        nmpt<const AccelerationPassType_> accelerationPass;
        s32 subpassIndex;
    };
}
