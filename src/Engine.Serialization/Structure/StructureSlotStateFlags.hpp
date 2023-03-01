#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Flag.hpp>

namespace hg::engine::serialization {
    enum class StructureSlotStateFlag : u8 {
        eUndefined = 0x0,
        /**/
        eMutable = 0x1,
        eImmutable = 0x2,
        /**/
        eDirty = 0x4
    };

    typedef Flag<StructureSlotStateFlag, u8> StructureSlotStateFlags;
}
