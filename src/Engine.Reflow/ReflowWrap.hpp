#pragma once

#include <Engine.Common/Types.hpp>

namespace hg::engine::reflow {
    enum class ReflowWrap: u8 {
        eNone = 0u,
        eNoWrap = 0u,
        eWrap = 1u
    };
}
