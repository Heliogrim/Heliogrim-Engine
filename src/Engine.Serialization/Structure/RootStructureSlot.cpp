#include "RootStructureSlot.hpp"

#include <Engine.Common/Make.hpp>
#include "../Archive/Archive.hpp"
#include "StructureSlotState.hpp"

using namespace ember::engine::serialization;
using namespace ember;

RootStructureSlot::RootStructureSlot(mref<sptr<StructureSlotState>> state_) :
    RecordScopedSlot(ScopedSlotState {
        ScopedSlotStateFlag::eUndefined,
        *this,
        state_
    }) {}

RootStructureSlot::RootStructureSlot(const non_owning_rptr<Archive> archive_) :
    RootStructureSlot(makeRootState(archive_)) {}

sptr<StructureSlotState> RootStructureSlot::makeRootState(const non_owning_rptr<Archive> archive_) {
    return make_sptr<StructureSlotState>(archive_);
}
