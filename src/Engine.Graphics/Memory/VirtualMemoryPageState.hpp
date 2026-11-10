#pragma once
#include <Engine.Common/Flag.hpp>
#include <Engine.Common/Types.hpp>

namespace hg::engine::gfx {
    enum class VirtualMemoryPageState: u8 {
        eUnloaded = 0x0,
        eTransient = 0x1,
        eLoaded = 0x2
    };

    typedef Flag<VirtualMemoryPageState, hg::u8> VirtualMemoryPageStates;
}
