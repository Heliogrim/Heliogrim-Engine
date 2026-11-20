#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::resource {
    enum class PackageEndianness : u8 {
        eBigEndian = 0xF0,
        eLittleEndian = 0x0F
    };
}
