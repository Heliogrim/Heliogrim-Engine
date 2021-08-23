#pragma once
#include "../Flag.hpp"

namespace ember {
    enum class CoreStateFlag {
        eUninitialized = 0x00,
        ePreInit = 0x01,
        eInit = 0x02,
        ePostInit = 0x02,
        eReady = 0x03,
        eDestroy = 0x04
    };

    using CoreStateFlags = Flag<CoreStateFlag>;
}
