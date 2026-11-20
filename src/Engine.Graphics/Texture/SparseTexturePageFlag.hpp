#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
    enum class SparseTexturePageFlag : u8 {
        eNone = 0x0,
        //
        eOpaqueTail = 0x1
    };

    typedef Flag<SparseTexturePageFlag, ::hg::u8> SparseTexturePageFlags;
}
