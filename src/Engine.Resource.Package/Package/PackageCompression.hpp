#pragma once

#include <Engine.Common/Types.hpp>

namespace ember::engine::resource {
    enum class PackageCompression : u16 {
        eNone = 0x0,
        eZLib = 0x1,
    };
}
