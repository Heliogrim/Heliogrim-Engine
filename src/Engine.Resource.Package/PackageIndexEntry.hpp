#pragma once

#include <Engine.Common/Types.hpp>

#include "__fwd.hpp"

namespace ember::engine::resource {
    struct PackageIndexEntry {
        u64 offset;
        u64 size;
    };
}
