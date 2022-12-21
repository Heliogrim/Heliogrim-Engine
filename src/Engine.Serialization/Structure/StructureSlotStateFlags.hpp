#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Flag.hpp>

namespace ember::engine::serialization {
    enum class StructureSlotStateFlag : u8 {
        eUndefined = 0x0,
        /**/
        eImmutable,
        //eDirty
    };

    typedef Flag<StructureSlotStateFlag, u8> StructureSlotStateFlags;
}
