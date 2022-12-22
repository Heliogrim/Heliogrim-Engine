#pragma once

#include "ScopedStructureSlot.hpp"

namespace ember::engine::serialization {
    class SeqScopedSlot final :
        ScopedSlot {
    public:
        using this_type = SeqScopedSlot;
        using underlying_type = ScopedSlot;

    public:
        SeqScopedSlot(mref<ScopedSlotState> scopedState_, mref<StructureSlotState> state_);

        ~SeqScopedSlot() override;
    };
}
