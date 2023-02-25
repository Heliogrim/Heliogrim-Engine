#pragma once

#include <Engine.Common/Flag.hpp>
#include <Engine.Common/Types.hpp>

namespace ember::engine::gfx {
    enum class VirtualTexturePageFlag : u8 {
        eNone = 0x0,
        //
        eOpaqueTail = 0x1
    };

    typedef Flag<VirtualTexturePageFlag, ::ember::u8> VirtualTexturePageFlags;
}
