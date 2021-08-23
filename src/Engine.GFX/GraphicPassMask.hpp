#pragma once
#include <Engine.Common/Types.hpp>

namespace ember::engine::gfx {
    enum class GraphicPassMask: u8 {
        eDepthPass,
        eLightPass,
        eProbePass,
        ePbrPass,
        eFinalPass
    };
}
