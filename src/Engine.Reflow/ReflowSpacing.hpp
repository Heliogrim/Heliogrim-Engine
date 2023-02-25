#pragma once

#include <Engine.Common/Types.hpp>

namespace ember::engine::reflow {
    enum class ReflowSpacing : u8 {
        eSpaceAround = 0ui8,
        eSpaceBetween = 1ui8,
        eSpaceEvenly = 2ui8,
        //
        eStart = 5ui8,
        eEnd = 6ui8,
    };
}
