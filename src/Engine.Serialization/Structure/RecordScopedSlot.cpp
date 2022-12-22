#include "RecordScopedSlot.hpp"

#include "SeqScopedSlot.hpp"
#include "StringScopedSlot.hpp"
#include "StructScopedSlot.hpp"
#include "StructureSlotState.hpp"

#include "../Archive/Archive.hpp"

using namespace ember::engine::serialization;
using namespace ember;

RecordScopedSlot::RecordScopedSlot(cref<ScopedSlotState> state_) :
    ScopedStructureSlotBase(state_) {
    ensureEntered(not _state.isScopedImmutable());
}

RecordScopedSlot::RecordScopedSlot(mref<ScopedSlotState> state_) :
    ScopedStructureSlotBase(_STD move(state_)) {
    ensureEntered(not _state.isScopedImmutable());
}

RecordScopedSlot::~RecordScopedSlot() = default;

void RecordScopedSlot::enter(const bool mutating_) {}

void RecordScopedSlot::leave(const bool mutating_) {}

const StructScopedSlot RecordScopedSlot::intoStruct() const {
    _state.flags |= ScopedSlotStateFlag::eImmutable;
    return StructScopedSlot { _STD move(_state) };
}

StructScopedSlot RecordScopedSlot::intoStruct() {
    return StructScopedSlot { _STD move(_state) };
}

const SeqScopedSlot RecordScopedSlot::intoSeq() const {
    _state.flags |= ScopedSlotStateFlag::eImmutable;
    return SeqScopedSlot { _STD move(_state) };
}

const StringScopedSlot RecordScopedSlot::intoString() const {
    _state.flags |= ScopedSlotStateFlag::eImmutable;
    return StringScopedSlot { _STD move(_state) };
}

StringScopedSlot RecordScopedSlot::intoString() {
    return StringScopedSlot { _STD move(_state) };
}
