#include "ScopedSlotGuard.hpp"

#include "ScopedStructureSlot.hpp"

using namespace hg::engine::serialization;
using namespace hg;

ScopedSlotGuard::ScopedSlotGuard(
	const non_owning_rptr<const ScopedSlot> scopedSlot_,
	const ScopedSlotGuardMode mode_
) :
	_mode(mode_),
	_scopedSlot(scopedSlot_) {

	auto* const slot = _scopedSlot->slot();
	if (_mode == ScopedSlotGuardMode::eRead) {
		slot->readHeader();

	} else {
		slot->writeHeader();
	}

	slot->enter();
}

ScopedSlotGuard::~ScopedSlotGuard() {

	auto* const slot = _scopedSlot->slot();
	slot->leave();

	if (_mode == ScopedSlotGuardMode::eRead) {
		// __noop();

	} else {
		slot->writeHeader();
	}
}
