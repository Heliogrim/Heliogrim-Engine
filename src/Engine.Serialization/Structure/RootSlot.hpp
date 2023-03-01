#pragma once

#include "RecordSlot.hpp"

namespace hg::engine::serialization {
    class RootSlot final :
        public RecordSlot {
    public:
        using this_type = RecordSlot;

    protected:
        RootSlot(cref<StructureSlotState> state_);

        RootSlot(mref<StructureSlotState> state_);

    public:
        RootSlot(const non_owning_rptr<Archive> archive_);

    public:
        [[nodiscard]] static StructureSlotState makeRootState(const non_owning_rptr<Archive> archive_);
    };
}
