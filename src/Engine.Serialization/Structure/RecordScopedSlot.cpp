#include "RecordScopedSlot.hpp"

#include <Engine.Common/Make.hpp>

#include "RecordSlot.hpp"
#include "SeqScopedSlot.hpp"
#include "StructScopedSlot.hpp"

using namespace hg::engine::serialization;
using namespace hg;

RecordScopedSlot::RecordScopedSlot(mref<ScopedSlotState> scopedState_, mref<sptr<RecordSlot>> slot_) :
    ScopedSlot(_STD move(scopedState_), _STD move(slot_)) {}

RecordScopedSlot::RecordScopedSlot(mref<ScopedSlotState> scopedState_, mref<StructureSlotState> state_) :
    ScopedSlot(_STD move(scopedState_), make_sptr<RecordSlot>(_STD move(state_))) {}

RecordScopedSlot::~RecordScopedSlot() = default;

StructScopedSlot RecordScopedSlot::intoStruct() {
    return StructScopedSlot { _STD move(_state), _STD move(_slot->getState()) };
}

StructScopedSlot RecordScopedSlot::asStruct() const {

    StructureSlotState aliasState { _slot->getState() };
    aliasState.flags = StructureSlotStateFlag::eImmutable;

    return StructScopedSlot(ScopedSlotState { _state }, _STD move(aliasState));
}

SeqScopedSlot RecordScopedSlot::asSeq() const {

    StructureSlotState aliasState { _slot->getState() };
    aliasState.flags = StructureSlotStateFlag::eImmutable;

    return SeqScopedSlot(ScopedSlotState { _state }, _STD move(aliasState));
}

SeqScopedSlot RecordScopedSlot::intoSeq() {
    return SeqScopedSlot { _STD move(_state), _STD move(_slot->getState()) };
}

StringScopedSlot RecordScopedSlot::intoString() {
    return StringScopedSlot { _STD move(_state), _STD move(_slot->getState()) };
}
