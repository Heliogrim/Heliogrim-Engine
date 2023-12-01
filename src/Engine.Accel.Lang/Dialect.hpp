#pragma once

#include <Engine.Common/Types.hpp>

namespace hg::engine::accel::lang {
    enum class Dialect : u8 {
        eAccelGlsl,
        /* Warning: Emit Only */
        eVulkanGlsl330,
        eVulkanGlsl460
    };
}
