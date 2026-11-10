#pragma once

#include "__fwd.hpp"

namespace hg::engine::gfx::memory {
    enum AllocationResult : u8 {
        eUndefined = 0x0,
        eSuccess = 0x1,
        //
        eFailed = 0x2,
        eOutOfMemory = 0x3
    };
}
