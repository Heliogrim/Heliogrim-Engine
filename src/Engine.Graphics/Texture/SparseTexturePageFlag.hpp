#pragma once

#include <Engine.Common/Flag.hpp>
#include <Engine.Common/Types.hpp>

namespace hg::engine::gfx {
    enum class SparseTexturePageFlag : u8 {
        eNone = 0x0,
        //
        eOpaqueTail = 0x1
    };

    typedef Flag<SparseTexturePageFlag, ::hg::u8> SparseTexturePageFlags;
}
