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

StructScopedSlot RecordScopedSlot::intoStruct() const {
    return StructScopedSlot { _STD move(_state), _STD move(_slot->getState()) };
}

SeqScopedSlot RecordScopedSlot::intoSeq() const {
    return SeqScopedSlot { _STD move(_state), _STD move(_slot->getState()) };
}

StringScopedSlot RecordScopedSlot::intoString() const {
    return StringScopedSlot { _STD move(_state), _STD move(_slot->getState()) };
}
