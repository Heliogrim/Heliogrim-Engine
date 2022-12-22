#include "ScopedStructureSlot.hpp"

#include "../Archive/Archive.hpp"

using namespace ember::engine::serialization;
using namespace ember;

ScopedSlot::ScopedSlot(
    mref<ScopedSlotState> state_,
    mref<sptr<StructureSlotBase>> slot_
) :
    _state(_STD move(state_)),
    _slot(_STD move(slot_)) {}

cref<ScopedSlotState> ScopedSlot::getScopedState() const noexcept {
    return _state;
}

ref<ScopedSlotState> ScopedSlot::getScopedState() noexcept {
    return _state;
}

const non_owning_rptr<StructureSlotBase> ScopedSlot::slot() const noexcept {
    return _slot.get();
}

#if FALSE
void ScopedStructureSlotBase::enter(const bool mutating_) {

    if (_state.offset != -1i64) {
        reenter(mutating_);
    } else {
        _state.offset = _state.rootState->getArchive()->tell();
    }

    if (mutating_) {
        _state.flags |= ScopedSlotStateFlag::eDirty;
    }
}

void ScopedStructureSlotBase::leave(const bool mutating_) {

    if (not mutating_) {
        return;
    }

    if (_state.size != 0i64) {
        // reentered slot or repeated leave propagation
        _state.size = _STD max(_state.size, _STD abs(_state.rootState->getArchive()->tell() - _state.offset));
    } else {
        _state.size = _STD abs(_state.rootState->getArchive()->tell() - _state.offset);
    }

    _state.flags.unwrap &= ~(static_cast<ScopedSlotStateFlags::value_type>(ScopedSlotStateFlag::eDirty));
}
#endif
