#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Guid.hpp>

#include "PackageVersion.hpp"
#include "PackageCompression.hpp"
#include "PackageEndianness.hpp"

namespace ember::engine::resource {
    #pragma pack(push)
    #pragma pack(1)
    struct PackageHeader {
        // 0x00
        u8 magicBytes[6];
        u8 magicVersion;
        PackageEndianness endianness;
        // 0x08
        PackageVersion version;
        // 0x10
        u8 guid[16];// ~ ::ember::Guid is not trivial
        // 0x20
        PackageCompression compression;
        u16 __reserved__;
        // 0x24
        u64 packageSize;
        u64 inflatedSize;
        // 0x34
        u64 indexOffset;
        u64 indexSize;
        // 0x44
        u64 metaOffset;
        u64 metaSize;
        // 0x54
        u64 compDataOffset;
        u64 compDataSize;
        // 0x64
    };
    #pragma pack(pop)

    static_assert(_STD is_trivial_v<PackageHeader>);
    static_assert(sizeof(PackageHeader) == 0x64);
}
