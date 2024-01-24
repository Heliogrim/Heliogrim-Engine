#include "RecordSlot.hpp"

using namespace hg::engine::serialization;
using namespace hg;

RecordSlot::RecordSlot(cref<StructureSlotState> state_) :
    StructureSlotBase(state_) {
    _state.header = StructureSlotHeader::from<StructureSlotType::eRecord>();
}

RecordSlot::RecordSlot(mref<StructureSlotState> state_) :
    StructureSlotBase(std::move(state_)) {
    _state.header = StructureSlotHeader::from<StructureSlotType::eRecord>();
}

RecordSlot::~RecordSlot() {
    if (_state.flags & StructureSlotStateFlag::eDirty && _state.root) {
        writeHeader();
        leave();
    }
}

StructureSlotType RecordSlot::getSlotType() const noexcept {
    return StructureSlotType::eRecord;
}

bool RecordSlot::validateType() const noexcept {
    return _state.header.type == StructureSlotType::eRecord;
}
