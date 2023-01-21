#pragma once

#include <Engine.Common/Types.hpp>

namespace ember::engine::resource {
    enum class PackageEndianness : u8 {
        eBigEndian = 0xF0,
        eLittleEndian = 0x0F
    };
}
