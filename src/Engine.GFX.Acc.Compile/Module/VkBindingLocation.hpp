#pragma once

#include <Engine.Common/Types.hpp>

namespace hg::engine::gfx::acc {
    struct VkBindingLocation final {
        s32 set;
        s32 location;
    };
}
