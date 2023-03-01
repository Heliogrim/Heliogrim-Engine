#pragma once
#include "../Flag.hpp"

namespace hg {
    enum class CoreStateFlag : u8 {
        eUninitialized = 0x00,
        ePreInit = 0x01,
        eInit = 0x02,
        ePostInit = 0x02,
        eReady = 0x03,
        eRunning = 0x04,
        eStopped = 0x05,
        eDestroy = 0x06
    };

    using CoreStateFlags = Flag<CoreStateFlag>;
}
