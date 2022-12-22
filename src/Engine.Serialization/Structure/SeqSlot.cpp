#include "SeqSlot.hpp"

using namespace ember::engine::serialization;
using namespace ember;

SeqSlot::SeqSlot(cref<StructureSlotState> state_) :
    StructureSlotBase(state_) {
    _state.header = StructureSlotHeader::from<StructureSlotType::eSeq>();
}

SeqSlot::SeqSlot(mref<StructureSlotState> state_) :
    StructureSlotBase(_STD move(state_)) {
    _state.header = StructureSlotHeader::from<StructureSlotType::eSeq>();
}

bool SeqSlot::validateType() const noexcept {
    return _state.header.type == StructureSlotType::eSeq;
}
