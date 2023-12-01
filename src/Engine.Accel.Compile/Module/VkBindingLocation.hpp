#pragma once

#include <Engine.Common/Types.hpp>

namespace hg::engine::accel {
    struct VkBindingLocation final {
        s32 set;
        s32 location;
    };
}
