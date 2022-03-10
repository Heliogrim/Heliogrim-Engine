#pragma once
#include <Engine.Common/Flag.hpp>
#include <Engine.Common/Types.hpp>

namespace ember::engine::gfx {

    enum class MemoryProperty {
        eUndefined = 0x0,
        //
        eDeviceLocal = 0x1,
        eHostVisible = 0x2,
        eHostCoherent = 0x4,
        eHostCached = 0x8,
        eLazilyAllocated = 0x10,
        eProtected = 0x20
    };

    typedef Flag<MemoryProperty, ember::u8> MemoryProperties;
}
