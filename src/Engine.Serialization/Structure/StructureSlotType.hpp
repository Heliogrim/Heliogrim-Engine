#pragma once

#include <Engine.Common/Types.hpp>

namespace ember::engine::serialization {
    enum class StructureSlotType : u8 {
        eUndefined = 0x0,
        /* Integral ( signed ~ 0x1 ) */
        eBool = 0x1,
        eU8 = (0x2),
        eU16 = (0x4),
        eU32 = (0x6),
        eU64 = (0x8),
        eS8 = eU8 | 0x1,
        eS16 = eU16 | 0x1,
        eS32 = eU32 | 0x1,
        eS64 = eU64 | 0x1,
        /**/
        eFloat = 0xA,
        eDouble = 0xB,
        /**/
        eString = 0xF,
        /**/
        eSlice = 0x10,
        eMap = 0x20,
        eMapEntry = 0x30,
        /**/
        eRecord = 0xA0,
        eStruct = 0xB0,
        eSeq = 0xC0
    };
}
