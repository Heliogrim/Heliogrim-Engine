#pragma once

#include <Engine.Common/Types.hpp>

namespace ember::engine::reflow {
    enum class EventResponse : u8 {
        eUnhandled = 0x0,
        eHandled = 0x1,
        //
        eConsumed = eHandled | 0x2
    };
}
