#include "RecordSlot.hpp"

using namespace ember::engine::serialization;
using namespace ember;

RecordSlot::RecordSlot(cref<StructureSlotState> state_) :
    StructureSlotBase(state_) {
    _state.header = StructureSlotHeader::from<StructureSlotType::eRecord>();
}

RecordSlot::RecordSlot(mref<StructureSlotState> state_) :
    StructureSlotBase(_STD move(state_)) {
    _state.header = StructureSlotHeader::from<StructureSlotType::eRecord>();
}

RecordSlot::~RecordSlot() {
    if (_state.flags & StructureSlotStateFlag::eDirty && _state.root) {
        writeHeader();
        leave();
    }
}

bool RecordSlot::validateType() const noexcept {
    return _state.header.type == StructureSlotType::eRecord;
}

void RecordSlot::leave() {
    if (_state.flags & StructureSlotStateFlag::eMutable && _state.parent) {
        _state.parent->feedback(this);
    }

    _state.flags.unwrap &= ~(static_cast<StructureSlotStateFlags::value_type>(StructureSlotStateFlag::eDirty));
}
