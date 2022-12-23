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

void ScopedSlot::enterSlot() const {
    _slot->enter();
}

void ScopedSlot::leaveSlot() const {
    _slot->leave();
}
