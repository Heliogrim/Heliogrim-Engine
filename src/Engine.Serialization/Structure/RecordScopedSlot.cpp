#include "RecordScopedSlot.hpp"

#include "SeqScopedSlot.hpp"
#include "StringScopedSlot.hpp"
#include "StructScopedSlot.hpp"

using namespace ember::engine::serialization;
using namespace ember;

RecordScopedSlot::RecordScopedSlot(cref<ScopedSlotState> state_) :
    ScopedStructureSlotBase(state_) {}

RecordScopedSlot::RecordScopedSlot(mref<ScopedSlotState> state_) :
    ScopedStructureSlotBase(_STD move(state_)) {}

RecordScopedSlot::~RecordScopedSlot() = default;

const StructScopedSlot RecordScopedSlot::intoStruct() const {
    return StructScopedSlot { _STD move(_state) };
}

StructScopedSlot RecordScopedSlot::intoStruct() {
    return StructScopedSlot { _STD move(_state) };
}

const SeqScopedSlot RecordScopedSlot::intoSeq() const {
    return SeqScopedSlot { _STD move(_state) };
}

const StringScopedSlot RecordScopedSlot::intoString() const {
    return StringScopedSlot { _STD move(_state) };
}
