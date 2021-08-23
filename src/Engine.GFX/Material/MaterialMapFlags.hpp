#pragma once
#include <Engine.Common/Flag.hpp>

namespace clockwork::__internal::gfx {

    enum class MaterialMapFlagBits : u16 {
        eAlbedo = 0b1 << 1,
        eAlpha = 0b1 << 2,
        eAo = 0b1 << 3,
        eDisplacement = 0b1 << 4,
        eEmission = 0b1 << 5,
        eMetalness = 0b1 << 6,
        eNormal = 0b1 << 7,
        eRoughness = 0b1 << 8,
        eSpecular = 0b1 << 9,
        //
        eAll = 0xFFFF
    };

    typedef __internal::types::Flag<MaterialMapFlagBits, u16> MaterialMapFlags;
}
