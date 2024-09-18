#include "ScopedStructureSlot.hpp"

using namespace hg::engine::serialization;
using namespace hg;

ScopedSlot::ScopedSlot(
	mref<ScopedSlotState> state_,
	mref<sptr<StructureSlotBase>> slot_
) :
	_state(std::move(state_)),
	_slot(std::move(slot_)) {}

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
