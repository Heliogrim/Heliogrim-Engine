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

    public:
        [[nodiscard]] RecordScopedSlot addRecordSlot();

        [[nodiscard]] RecordScopedSlot getRecordSlot(const u64 index_) const;
    };
}
