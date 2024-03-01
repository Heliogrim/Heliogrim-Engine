#pragma once

#include <Engine.Common/Types.hpp>

namespace hg::engine::reflow {
    enum class ReflowSpacing : u8 {
        eSpaceAround = 0u,
        eSpaceBetween = 1u,
        eSpaceEvenly = 2u,
        //
        eStart = 5u,
        eEnd = 6u,
    };
}
