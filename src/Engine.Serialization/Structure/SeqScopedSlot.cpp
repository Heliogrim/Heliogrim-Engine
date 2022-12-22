#include "SeqScopedSlot.hpp"

#include "RecordScopedSlot.hpp"

using namespace ember::engine::serialization;
using namespace ember;

SeqScopedSlot::SeqScopedSlot(cref<ScopedSlotState> state_) :
    ScopedStructureSlotBase(state_) {
    ensureEntered(not _state.isScopedImmutable());
}

SeqScopedSlot::SeqScopedSlot(mref<ScopedSlotState> state_) :
    ScopedStructureSlotBase(_STD move(state_)) {
    ensureEntered(not _state.isScopedImmutable());
}

SeqScopedSlot::~SeqScopedSlot() = default;

const RecordScopedSlot SeqScopedSlot::getSeqEntry(const size_t index_) const {
    ScopedSlotState state {
        ScopedSlotStateFlag::eImmutable,
        *this,
        _state.rootState
    };
    return RecordScopedSlot { _STD move(state) };
}
