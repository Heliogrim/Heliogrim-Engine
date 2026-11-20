#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
    enum class VirtualMemoryPageState: u8 {
        eUnloaded = 0x0,
        eTransient = 0x1,
        eLoaded = 0x2
    };

    typedef Flag<VirtualMemoryPageState, hg::u8> VirtualMemoryPageStates;
}
