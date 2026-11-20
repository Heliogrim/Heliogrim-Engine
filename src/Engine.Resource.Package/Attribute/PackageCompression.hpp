#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::resource {
    enum class PackageCompression : u16 {
        eNone = 0x0,
        eZLib = 0x1,
    };
}
