#pragma once

#include <Engine.Common/Types.hpp>

namespace hg::engine::reflow {
    enum class ReflowType : u8 {
        eFlexRow = 0u,
        eFlexCol = 1u,
        eGrid = 2u
    };
}
