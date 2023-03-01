#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Flag.hpp>

namespace hg::engine::serialization {
    struct StructureSlotState;
    class StructureSlotBase;
}

namespace hg::engine::serialization {
    enum class ScopedSlotStateFlag : bool {
        eUndefined = false,
        /**/
        eClean = eUndefined,
        eDirty = true
    };

    typedef Flag<ScopedSlotStateFlag, bool> ScopedSlotStateFlags;

    struct ScopedSlotState {
        ScopedSlotStateFlags flags;
    };
}
