#pragma once

#include <Engine.Common/Types.hpp>

namespace ember::engine::gfx {
    enum class RenderEnqueueResult : u8 {
        eSuccess = 0x0,
        eFailed = 0x1,
        //
        eFailedChanged = 0x2
    };
}
