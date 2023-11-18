#pragma once

#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "__fwd.hpp"

namespace hg::driver::vk {
    template <typename AccelerationPassType_>
    struct SubpassInfo {
        nmpt<const AccelerationPassType_> accelerationPass;
        s32 subpassIndex;
    };
}
