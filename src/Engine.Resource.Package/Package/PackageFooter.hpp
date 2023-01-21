#pragma once

#include <type_traits>
#include <Engine.Common/Types.hpp>

#include "PackageEndianness.hpp"

namespace ember::engine::resource {
    #pragma pack(push)
    #pragma pack(1)
    struct PackageFooter {
        // 0x00
        u8 crc32[8];
        u8 __reserved__[8];
        // 0x10
        u8 magicBytes[6];
        u8 magicVersion;
        PackageEndianness endianess;
        // 0x18
    };
    #pragma pack(pop)

    static_assert(_STD is_trivial_v<PackageFooter>);
    static_assert(sizeof(PackageFooter) == 0x18);
}
