#pragma once

#include <Engine.Common/Collection/Vector.hpp>

namespace hg::engine::accel {
    enum class SpirvWord : u32 {};

    using SpirvByteCode = Vector<SpirvWord>;
}
