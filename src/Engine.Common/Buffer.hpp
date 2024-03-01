#pragma once

#include "Types.hpp"

namespace hg {
    struct Buffer {
        u64 size = 0uLL;
        u64 align = 0uLL;

        void* mem = nullptr;
    };
}
