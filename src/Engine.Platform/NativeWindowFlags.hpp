#pragma once

#include <Engine.Common/Flag.hpp>
#include <Engine.Common/Types.hpp>

namespace ember::engine::platform {
    enum class NativeWindowFlagBits : u8 {
        eNone = 0x0,
        //
        eFullScreen = 0x1,
        eBorderless = 0x2,
        eShown = 0x4,
        eHidden = 0x8,
        //
        eMovable = 0x10,
        eResizable = 0x20,
        eMinimized = 0x40,
        eMaximized = 0x80
    };

    typedef Flag<NativeWindowFlagBits, u8> NativeWindowFlags;
}
