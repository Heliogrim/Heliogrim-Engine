#pragma once

#include <Engine.Common/Types.hpp>

namespace ember::engine::reflow {

    enum class ReflowType : u8 {
        eFlexRow = 0ui8,
        eFlexCol = 1ui8,
        eGrid = 2ui8
    };

}
