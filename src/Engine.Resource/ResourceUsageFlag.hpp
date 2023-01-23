#pragma once
#include <Engine.Common/Flag.hpp>

namespace ember::engine::resource {
    enum class ResourceUsageFlag {
        eNone = 0b0000,
        eDefault = 0b0001,
        eRead = 0b0100,
        eWrite = 0b1000,
        eAll = 0b1111
    };

    using ResourceUsageFlags = Flag<ResourceUsageFlag>;
}
