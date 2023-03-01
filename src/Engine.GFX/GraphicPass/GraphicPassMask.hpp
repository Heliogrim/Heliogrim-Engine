#pragma once
#include <Engine.Common/Types.hpp>

namespace hg::engine::gfx {
    enum class GraphicPassMask: u8 {
        eDepthPass,
        eLightPass,
        eProbePass,
        eSkyPass,
        eMainPass,
        eFinalPass
    };
}
