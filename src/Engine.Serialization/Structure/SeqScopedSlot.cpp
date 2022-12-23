#include "SeqScopedSlot.hpp"

#include <Engine.Common/Make.hpp>

#include "RecordScopedSlot.hpp"
#include "SeqSlot.hpp"

using namespace ember::engine::serialization;
using namespace ember;

SeqScopedSlot::SeqScopedSlot(mref<ScopedSlotState> scopedState_, mref<StructureSlotState> state_) :
    ScopedSlot(_STD move(scopedState_), make_sptr<SeqSlot>(_STD move(state_))) {

    if (not _slot) {
        return;
    }

    if (_state.flags & ScopedSlotStateFlag::eDirty) {
        _slot->writeHeader();
    } else {
        _slot->readHeader();
    }

    _slot->enter();
}

SeqScopedSlot::~SeqScopedSlot() {

    if (not _slot) {
        return;
    }

    _slot->leave();

    if (_state.flags & ScopedSlotStateFlag::eDirty) {
        _slot->writeHeader();
    }
}

RecordScopedSlot SeqScopedSlot::addRecordSlot() {

    auto record = static_cast<ptr<SeqSlot>>(slot())->addRecord();
    ScopedSlotState scopedState { ScopedSlotStateFlag::eDirty };

    return RecordScopedSlot { _STD move(scopedState), _STD move(record) };

}

RecordScopedSlot SeqScopedSlot::getRecordSlot(const u64 index_) const {

    auto record = static_cast<ptr<SeqSlot>>(slot())->getRecord(index_);
    ScopedSlotState scopedState { ScopedSlotStateFlag::eClean };

    return RecordScopedSlot { _STD move(scopedState), _STD move(record) };
}
