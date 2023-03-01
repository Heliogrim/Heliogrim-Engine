#pragma once
#include <Engine.Common/Types.hpp>

namespace hg::engine::gfx {
    enum class InputRate : u8 {
        ePerVertex = 0x0,
        ePerInstance = 0x1
    };
}
