#pragma once

#include <Engine.Common/Types.hpp>

namespace ember::engine::reflow {
    enum class ReflowWrap: u8 {
        eNone = 0ui8,
        eNoWrap = 0ui8,
        eWrap = 1ui8
    };
}
