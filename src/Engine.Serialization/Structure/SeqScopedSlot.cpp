#include "SeqScopedSlot.hpp"

#include <Engine.Common/Make.hpp>

#include "RecordScopedSlot.hpp"
#include "SeqSlot.hpp"

using namespace hg::engine::serialization;
using namespace hg;

SeqScopedSlot::SeqScopedSlot(mref<ScopedSlotState> scopedState_, mref<StructureSlotState> state_) :
    ScopedSlot(std::move(scopedState_), make_sptr<SeqSlot>(std::move(state_))) {

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

    return RecordScopedSlot { std::move(scopedState), std::move(record) };

}

RecordScopedSlot SeqScopedSlot::getRecordSlot(const u64 index_) const {

    auto record = static_cast<ptr<SeqSlot>>(slot())->getRecord(index_);
    ScopedSlotState scopedState { ScopedSlotStateFlag::eClean };

    return RecordScopedSlot { std::move(scopedState), std::move(record) };
}

s64 SeqScopedSlot::getRecordCount() const {
    return static_cast<ptr<SeqSlot>>(slot())->getRecordCount();
}
