#pragma once

#include "Types.hpp"

namespace hg {
    struct Buffer {
        u64 size = 0ui64;
        u64 align = 0ui64;

        void* mem = nullptr;
    };
}
