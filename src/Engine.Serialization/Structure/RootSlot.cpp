#include "RootSlot.hpp"

#include "../Archive/Archive.hpp"

using namespace ember::engine::serialization;
using namespace ember;

RootSlot::RootSlot(cref<StructureSlotState> state_) :
    RecordSlot(state_) {}

RootSlot::RootSlot(mref<StructureSlotState> state_) :
    RecordSlot(_STD move(state_)) {}

RootSlot::RootSlot(const non_owning_rptr<Archive> archive_) :
    RecordSlot(makeRootState(archive_)) {}

StructureSlotState RootSlot::makeRootState(const non_owning_rptr<Archive> archive_) {
    return StructureSlotState {
        .flags = StructureSlotStateFlag::eUndefined,
        .offset = archive_->tell(),
        .header = StructureSlotHeader {},
        .parent = nullptr,
        .root = make_sptr<RootStructureSlotState>(RootStructureSlotState {
            StructureSlotStateFlag::eUndefined,
            archive_->tell(),
            StructureSlotHeader {},
            nullptr,
            nullptr,
            archive_
        })
    };
}
