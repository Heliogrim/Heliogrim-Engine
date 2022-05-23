#pragma once

#include "__fwd.hpp"

namespace ember::engine::gfx::cache {

    enum CacheResult : u8 {
        eUndefined = 0x0,
        eHit = 0x1,
        //
        eMiss = 0x2
    };
}
